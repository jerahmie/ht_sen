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
    uint16_t raw_data;
    uint8_t sht15_status;
     
    printf("Testing gpio.\n");
    setup_rpi_gpio();

    sht15_status = read_status_register();
    printf("SHT15 STATUS: %d", (uint)sht15_status );
    conn_reset();
    
    sleep(1);
    raw_data = measure_rht(MEASURE_TEMP);    
    sleep(1);
    printf("SOT: %d\n", raw_data);
    raw_data = measure_rht(MEASURE_REL_HUM);

    printf("SORH: %d\n", raw_data);

    return 0;
}
