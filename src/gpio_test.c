/*
 * gpio_test.c
 * Test the ability to toggle GPIO22 and GPIO23
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "rpi_gpio.h"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

// I/O access
volatile unsigned *gpio;

int main()
{
    printf("Testing gpio.\n");
    setup_rpi_gpio();


    // Set GPIO pins 22 and 23 to output
    INP_GPIO(22);
    INP_GPIO(23);
    OUT_GPIO(22);
    OUT_GPIO(23);

    // Flash LEDs
    while (1)
    {
	GPIO_SET = 1<<22;
	sleep(1);
	GPIO_SET = 1<<23;
	sleep(1);
	GPIO_CLR = 1<<22;
	sleep(1);
	GPIO_CLR = 1<<23;
	sleep(1);
    }
    printf("Done.\n");

    return NO_ERROR;
}
