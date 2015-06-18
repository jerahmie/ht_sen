/*
 * gpio_test.c
 * Test the ability to toggle GPIO22 and GPIO23
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "rpi_gpio.h"
#include "htsen.h"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

// I/O access
volatile unsigned *gpio;

int main()
{
    int raw_data;
    printf("Testing gpio.\n");
    setup_rpi_gpio();

//    read_status_register();
//    write_status_register();
    transmission_start();
//    transmission_start();
//    raw_data = measure_rht(MEASURE_TEMP);
//    printf("Raw data: %s\n", raw_data);
    raw_data = measure_rht(MEASURE_REL_HUM);
    printf("Raw data: %s\n", raw_data);

    return 0;
}
