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
  //    uint16_t raw_data;
  //    uint8_t sht15_status;
  float temperature_C, relative_humidity, dew_point_C;
  int status;
//  printf("Testing gpio.\n");
  setup_rpi_gpio();
//  sleep(1);
//  soft_reset();
//  sleep(1);
  
  //    conn_reset();

  //    sht15_status = read_status_register();
  //    printf("SHT15 STATUS: %d\n", sht15_status );

  //raw_data = measure_rht(MEASURE_TEMP_CMD);    
  //    printf("SOT: %d\n", raw_data);
  //    raw_data = measure_rht(MEASURE_REL_HUM_CMD);
  //    printf("SORH: %d\n", raw_data);

  while (1)
  {
      status = get_measurements(&temperature_C, &relative_humidity, &dew_point_C);
      printf("Temp: %.3f C, RH: %.3f, DP: %.3f C\n", temperature_C,
	     relative_humidity, dew_point_C);
      sleep(600);
  }
  return 0;
}
