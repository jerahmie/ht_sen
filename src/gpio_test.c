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

// I/O access
volatile unsigned *gpio;

int main()
{
    uint16_t raw_data;
    uint8_t sht15_status;
     
    printf("Testing gpio.\n");
    setup_rpi_gpio();
//    soft_reset();
//    conn_reset();

//    sht15_status = read_status_register();
//    printf("SHT15 STATUS: %d\n", sht15_status );

    
//    sleep(1);
    raw_data = measure_rht(MEASURE_TEMP);    
    printf("SOT: %d\n", raw_data);
    raw_data = measure_rht(MEASURE_REL_HUM);
    printf("SORH: %d\n", raw_data);

    return 0;
}
