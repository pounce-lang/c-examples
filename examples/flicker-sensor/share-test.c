/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
// #include "pico/util/queue.h"
#include "pico/multicore.h"

// crude shared data between two cores
int32_t data = 1;

void core1_entry()
{
    while (1)
    {
        data += 1;
        printf("c1 see data: %d\n", data);
        sleep_ms(1000);
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(4000);
    printf("Hello, multicore_runner using int!\n");

    multicore_launch_core1(core1_entry);
    while (1)
    {
        sleep_ms(700);
        if (data >= 5)
        {
            printf("c0 set to 1\n");
            data = 1;
        }
        sleep_ms(500);
    }
}
