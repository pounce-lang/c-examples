#include <stdio.h>
// #include <stdlib.h>
#include "pico/stdlib.h"
#include <string.h>
#include "../../c-core/src/pounce.c"
#include "pico-history.c"

const uint LED_PIN = 25;

void exit_pounce(ps_instance_ptr stack, dictionary *wd)
{
    ps_clear(stack);
    free(stack);
    dictionary_del(wd);
    gpio_put(LED_PIN, 1);
    sleep_ms(1000);
    gpio_put(LED_PIN, 0);
    printf("exiting Pounce -\\_(*|*)_/-");
}

int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // gpio_put(LED_PIN, 1);
    // sleep_ms(5000);
    // gpio_put(LED_PIN, 0);
    // sleep_ms(1000);

    dictionary *wd = init_core_word_dictionary();

    char input_program[INPUT_SIZE] = 
    "0 TIME-stream [LED_PIN ON putIO] 1s 0.5s elapse [LED_PIN OFF putIO] 1 0.0 elapse";

    ps_instance_ptr stack = ps_init();
    parser_result_ptr pr = parse(0, input_program);

    if (pr && pr->pq)
    {

        while(1) {
            stack = purr(stack, pr->pq, wd);
            // ps_display(stack);
        }
        // if the while loop was to exit
        free(pr->pq);
        free(pr);
    }
    else
    {
        printf("failed to parse!");
        return 1;
    }
}
