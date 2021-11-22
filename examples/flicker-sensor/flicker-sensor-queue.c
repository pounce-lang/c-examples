// Sample from the ADC continuously at a particular sample rate
// compute an FFT over the data and scan the frequency domain data
// for significant power at some frequency.
//
// much of this code is from pico-examples/adc/dma_capture/dma_capture.c
// the FFT processing written by Alex Wulff (www.AlexWulff.com)
// the multi-core and pounce-lang by Nate Morse

#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "awulff-pico-playground/adc_fft/kiss_fftr.h"

// based on a 48MHz ADC clock speed
// and using successive-approximation (SA) 
// a minimum of 96 samples are taken, the fastest SA sampling is  
// set this to determine sample rate
// | clock-div |=| sample-freq |Hz|
// |   SA off  |=| 48,00,000   |Hz|
// | 96        |=| 500,000     |Hz|
// | 960       |=| 50,000      |Hz|
// | 9600      |=| 5,000       |Hz|
#define CLOCK_DIV 960
#define FSAMP 50000

// Channel 0 is GPIO26
#define CAPTURE_CHANNEL 0
#define LED_PIN 25

// BE CAREFUL: anything over about 9000 here will cause things
// to silently break. The code will compile and upload, but due
// to memory issues nothing will work properly
#define NSAMP 5000

// globals
dma_channel_config cfg;
uint dma_chan;
float freqs[NSAMP];
queue_t q;

void setup();
void sample(uint8_t *capture_buf);

void core_1_main()
{
  kiss_fft_cpx fft_freq[(NSAMP/2)];
  kiss_fft_cpx temp;
  while (true)
  {
    printf("from core-1: going to read from the queue");
    for (int i = 0; i < NSAMP / 2; i++)
    {
      queue_remove_blocking(&q, &temp);
      fft_freq[i] = temp;		
    }
    // compute power and calculate max freq component
    float max_power = 0;
    float avg_power = 0;
    int max_idx = 0;
    // any frequency bin over NSAMP/2 is aliased (nyquist sampling theorum)
    for (int i = 0; i < NSAMP / 2; i++)
    {
      float power = fft_freq[i].r * fft_freq[i].r + fft_freq[i].i * fft_freq[i].i;
      if (power > max_power)
      {
        max_power = power;
        max_idx = i;
      }
      avg_power += power / NSAMP;
    }

    float max_freq = freqs[max_idx];
    if (max_power > avg_power * 200)
    {
      printf("significant frequency: %6.1f Hz\t%6.0f\n", max_freq, (max_power / avg_power));
    }
    else
    {
      printf("\t\t\t\tno significant frequency\n");
    }
  }
}

int main()
{
  stdio_init_all();
  queue_init(&q, sizeof(kiss_fft_cpx), NSAMP);
  printf("sizeof(kiss_fft_cpx) %d", sizeof(kiss_fft_cpx));

  uint8_t cap_buf[NSAMP];
  kiss_fft_scalar fft_in[NSAMP]; // kiss_fft_scalar is a float
  kiss_fft_cpx fft_out[NSAMP];
  kiss_fftr_cfg cfg = kiss_fftr_alloc(NSAMP, false, 0, 0);
  
  // setup ports and outputs
  setup();

  multicore_launch_core1(core_1_main);

  while (true)
  {
    // get NSAMP samples at FSAMP
    sample(cap_buf);
    // fill fourier transform input while subtracting DC component
    uint64_t sum = 0;
    for (int i = 0; i < NSAMP; i++)
    {
      sum += cap_buf[i];
    }
    float avg = (float)sum / NSAMP;
    for (int i = 0; i < NSAMP; i++)
    {
      fft_in[i] = (float)cap_buf[i] - avg;
    }

    // compute fast fourier transform
    kiss_fftr(cfg, fft_in, fft_out);
    // send a sync of 3 (-1)s, then pipe out the fft_out
    kiss_fft_cpx temp;
    gpio_put(LED_PIN, 1);
    for (int i = 0; i < NSAMP / 2; i++)
    {
      temp = fft_out[i];
      queue_add_blocking(&q, &temp);
    }
    gpio_put(LED_PIN, 1);

  }

  // should never get here
  kiss_fft_free(cfg);
}

void sample(uint8_t *capture_buf)
{
  adc_fifo_drain();
  adc_run(false);

  dma_channel_configure(dma_chan, &cfg,
                        capture_buf,   // dst
                        &adc_hw->fifo, // src
                        NSAMP,         // transfer count
                        true           // start immediately
  );

  // gpio_put(LED_PIN, 1);
  adc_run(true);
  dma_channel_wait_for_finish_blocking(dma_chan);
  // gpio_put(LED_PIN, 0);
}

void setup()
{
//  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  adc_gpio_init(26 + CAPTURE_CHANNEL);

  adc_init();
  adc_select_input(CAPTURE_CHANNEL);
  adc_fifo_setup(
      true,  // Write each completed conversion to the sample FIFO
      true,  // Enable DMA data request (DREQ)
      1,     // DREQ (and IRQ) asserted when at least 1 sample present
      false, // We won't see the ERR bit because of 8 bit reads; disable.
      true   // Shift each sample to 8 bits when pushing to FIFO
  );

  // set sample rate
  adc_set_clkdiv(CLOCK_DIV);

  sleep_ms(1000);
  // Set up the DMA to start transferring data as soon as it appears in FIFO
  uint dma_chan = dma_claim_unused_channel(true);
  cfg = dma_channel_get_default_config(dma_chan);

  // Reading from constant address, writing to incrementing byte addresses
  channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
  channel_config_set_read_increment(&cfg, false);
  channel_config_set_write_increment(&cfg, true);

  // Pace transfers based on availability of ADC samples
  channel_config_set_dreq(&cfg, DREQ_ADC);

  // calculate frequencies of each bin
  float f_max = FSAMP / 2;
  float f_res = f_max / NSAMP;
  for (int i = 0; i < NSAMP; i++)
  {
    freqs[i] = f_res * i;
  }
}
