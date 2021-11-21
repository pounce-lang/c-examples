#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#define RE_0A 3
#define RE_0B 4
#define RE_1A 5
#define RE_1B 6
#define ENCODERS 2

// delta vector lookup (from binary pin history)
// 0000 0001 0010 0011 ->  0,  1, -1,  2
// 0100 0101 0110 0111 -> -1,  0,  2,  1
// 1000 1001 1010 1011 ->  1,  2,  0, -1
// 1100 1101 1110 1111 ->  2, -1,  1,  0
// note that 2 is an error code that could be intertrated as a skipped encoding
int8_t delta[] = {
    0, 1, -1, 2,
    -1, 0, 2, 1,
    1, 2, 0, -1,
    2, -1, 1, 0 };

uint32_t pr_time[ENCODERS];
uint8_t re[ENCODERS], pr[ENCODERS];
int8_t pr_delta[ENCODERS], ppr_delta[ENCODERS];
int32_t cp[ENCODERS];

bool core1_record_encoder(uint8_t i)
{
    bool sig_update = false; // is this a significant update or noise
    uint32_t now_time = time_us_32(), dt;
    dt = now_time - pr_time[i];
    uint8_t dpins = re[i] + (pr[i] << 2);
    int8_t current_delta = delta[dpins];
    int8_t two_deltas = pr_delta[i] + ppr_delta[i];
    if (current_delta == 2)
    {
        if (pr_delta[i] < 0 && ppr_delta[i] < 0)
        {
            current_delta *= -1;
        }
        else if (pr_delta[i] <= 0 || ppr_delta[i] <= 0) {
            current_delta = 0;
        }
#ifdef LOG_LEVEL_1
        printf("encoder %d double (%d %d %d)\n", i, current_delta, pr_delta[i], ppr_delta[i]);
#endif
    }
    else if (two_deltas > 2 && current_delta == -1) {
        current_delta = 3;
#ifdef LOG_LEVEL_1
        printf("encoder %d tripple +++3\n", i);
#endif
    }
    else if (two_deltas < -2 && current_delta == 1) {
        current_delta = -3;
#ifdef LOG_LEVEL_1
        printf("encoder %d tripple ---3\n", i);
#endif
    }
    cp[i] = cp[i] + (int32_t)current_delta;
    if (dt > 500)
    {
        sig_update = true;
    }
    pr[i] = re[i];
    pr_time[i] = now_time;
    if (sig_update) {
        ppr_delta[i] = pr_delta[i];
        pr_delta[i] = current_delta;
    }
    return sig_update;
}

void core1_main()
{
    bool significant_update;
    pr_time[0] = pr_time[1] = time_us_32();
    pr_delta[0] = pr_delta[1] = ppr_delta[0] = ppr_delta[1] = 0;
    cp[0] = cp[1] = 0;
    gpio_init(RE_0A);
    gpio_init(RE_0B);
    gpio_init(RE_1A);
    gpio_init(RE_1B);
    gpio_set_dir(RE_0A, GPIO_IN);
    gpio_set_dir(RE_0B, GPIO_IN);
    gpio_set_dir(RE_1A, GPIO_IN);
    gpio_set_dir(RE_1B, GPIO_IN);
    gpio_pull_up(RE_0A);
    gpio_pull_up(RE_0B);
    gpio_pull_up(RE_1A);
    gpio_pull_up(RE_1B);
    while (true)
    {
        re[0] = gpio_get(RE_0A) + (gpio_get(RE_0B) << 1);
        re[1] = gpio_get(RE_1A) + (gpio_get(RE_1B) << 1);
        significant_update = false;
        if (re[0] != pr[0])
        {
            significant_update |= core1_record_encoder(0);
        }
        if (re[1] != pr[1])
        {
            significant_update |= core1_record_encoder(1);
        }
        if (significant_update)
        {
            multicore_fifo_push_blocking(cp[0]);
            multicore_fifo_push_blocking(cp[1]);
        }
        else {
            sleep_ms(1);
        }
    }
}

int main() {
    uint32_t re_pos_0, re_pos_1;
    stdio_init_all();

    multicore_launch_core1(core1_main);
    while (1) {
        re_pos_0 = multicore_fifo_pop_blocking();
        re_pos_1 = multicore_fifo_pop_blocking();
        printf("core0 %d %d\n", re_pos_0, re_pos_1);
    }
}
