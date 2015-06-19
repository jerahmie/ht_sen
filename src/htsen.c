/*
 * htsen.c
 * Humidity and temperature sensor interface.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "rpi_gpio.h"
#include "htsen.h"

// I/O access
volatile unsigned *gpio;

int soft_reset()
{
    int i;
    struct timespec tim_r, tim_clk;
    tim_r.tv_sec = 0; tim_r.tv_nsec = TRISE_NS;
    tim_clk.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS;

    printf("Resetting SHT15...\n");
    transmission_start();

    INP_GPIO(SCK);
    INP_GPIO(DATA);
    OUT_GPIO(SCK);
    OUT_GPIO(DATA);

    for (i=0; i<8; i++)
    {
      if (SOFT_RESET&(128>>i))
        {
          GPIO_SET = 1<<DATA;
        }
      else
        {
          GPIO_CLR = 1<<DATA;
        }
      nanosleep(&tim_r, NULL);
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk, NULL);
      GPIO_CLR = 1<<SCK;

    }
    sleep(1);
    return 0;
}

int transmission_start()
{
  printf("Starting send command.\n");
  
  struct timespec tim_setup, tim_clk;
  tim_setup.tv_sec = 0; tim_setup.tv_nsec = TRISE_NS;
  tim_clk.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS;
  
  INP_GPIO(SCK);
  INP_GPIO(DATA);
  OUT_GPIO(SCK);
  OUT_GPIO(DATA);

  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_setup, NULL);
  GPIO_SET = 1<<DATA;
  nanosleep(&tim_clk, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  GPIO_CLR = 1<<DATA;
  nanosleep(&tim_clk, NULL);

  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  GPIO_SET = 1<<DATA;
  nanosleep(&tim_clk, NULL);

  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  GPIO_CLR = 1<<DATA;
  nanosleep(&tim_clk, NULL);
  
  return 0;
}

int conn_reset()
{
  // Reset communication with SHT15
  // Hold DATA line hight while toggling SCK nine or more times, followed by
  // transmission start sequence
  int index;
  struct timespec tim_clk, tim_clk_half;
  tim_clk.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS;
  tim_clk_half.tv_sec = 0; tim_clk_half.tv_nsec = TSCK_NS>>1;
  
  printf("Resetting connection...");
  // set SCK, DATA pins to output
  INP_GPIO(SCK);
  INP_GPIO(DATA);
  OUT_GPIO(SCK);
  OUT_GPIO(DATA);

  GPIO_SET = 1<<DATA;
  nanosleep(&tim_clk_half, NULL);
  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_clk_half, NULL);

  // toggle SCK 9 times
  for( index=0; index<9; index++)
    {
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk, NULL);
      GPIO_CLR = 1<<SCK;
      nanosleep(&tim_clk, NULL);
    }
  transmission_start();

  return 0;
}

uint8_t read_status_register()
{
  int i;
  uint16_t sen_ack;
  uint8_t status_register = 0;

  struct timespec tim_r, tim_clk, tim_clk_half;
  tim_r.tv_sec = 0; tim_r.tv_nsec=TRISE_NS;      // rise/fall time
  tim_clk.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS; // clock high/low time
  tim_clk_half.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS>>1; // clock high/low time
  
  printf("read_status_register...\n");
  transmission_start();
  
  // set SCK, DATA to output
//  INP_GPIO(SCK);
  //INP_GPIO(DATA);
  OUT_GPIO(SCK);
  OUT_GPIO(DATA);
  
  for (i=0; i<8; i++)
    {
      if (READ_STATUS_REG&(128>>i))
        {
          GPIO_SET = 1<<DATA;
        }
      else
        {
          GPIO_CLR = 1<<DATA;
        }
      nanosleep(&tim_r, NULL);
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk, NULL);
      GPIO_CLR = 1<<SCK;
      nanosleep(&tim_clk, NULL);
    }

  // switch DATA to input and read status register
  nanosleep(&tim_r, NULL);
  INP_GPIO(DATA);
  nanosleep(&tim_r, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim_clk_half, NULL);
  sen_ack = GET_GPIO(DATA);
  printf("sen_ack: %d\n", sen_ack);
  nanosleep(&tim_clk_half, NULL);
  if (sen_ack)
    {
      printf("[ERROR] Did not receive ACK from sensor! Exiting.\n");
      return -1;
    }

  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_clk, NULL);

  // read status register (8-bit)
  for(i=0; i<8; i++)
    {
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk_half, NULL);
      status_register = status_register <<1;
//      printf("%d\n", GET_GPIO(DATA));
      if ( GET_GPIO(DATA) ) {
	  status_register += 1;
      }
//	  printf("1");
//      } else {
//	  printf("0");
//      }

      nanosleep(&tim_clk_half, NULL);
      GPIO_CLR = 1<<SCK;
      nanosleep(&tim_clk, NULL);
    }
  GPIO_SET = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  
  return status_register;
}

int write_status_register(uint8_t status_register)
{
  int i;
  printf("write_status_register...\n");
  for (i=0; i<8; i++)
    {
      printf("%d", (WRITE_STATUS_REG&(128>>i))?1:0);
    }
  printf("\n");
  
  return 0;
}

uint16_t measure_rht( uint8_t measurement_type)
{
  int i;
  uint16_t measurement_raw = 0;
  uint16_t sen_ack;
  struct timespec tim_r, tim_clk, tim_clk_half;
  tim_r.tv_sec = 0; tim_r.tv_nsec=TRISE_NS;      // rise/fall time
  tim_clk.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS; // clock high/low time
  tim_clk_half.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS>>1; // clock high/low time

  if ( measurement_type == MEASURE_TEMP )
    {
      printf("Measuring temperature...\n");
    }
  else if ( measurement_type == MEASURE_REL_HUM )
    {
      printf("Measuring humidity...\n");
    }

  transmission_start();

  // set SCK, DATA to output
  OUT_GPIO(SCK);
  OUT_GPIO(DATA);
  
  for (i=0; i<8; i++)
    {
	if (measurement_type&(128>>i)) {
	    GPIO_SET = 1<<DATA;
	} else {
	    GPIO_CLR = 1<<DATA;
	}
        nanosleep(&tim_r, NULL);
	GPIO_SET = 1<<SCK;
	nanosleep(&tim_clk, NULL);
	GPIO_CLR = 1<<SCK;
    }
  
  // switch DATA to input to read results
  INP_GPIO(DATA);
  nanosleep(&tim_clk, NULL);
	
  //nanosleep(&tim_r, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim_clk_half, NULL);
  sen_ack = GET_GPIO(DATA);
  nanosleep(&tim_clk_half, NULL);
  if (sen_ack)
    {
      printf("[ERROR] Did not receive ACK from sensor! Exiting.\n");
      return -1;
    }
  
  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_clk, NULL);

  // read MSb
  for(i=0; i<8; i++)
    {
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk_half, NULL);
      measurement_raw = measurement_raw<<1;
      if (GET_GPIO(DATA)){
	  measurement_raw += 1;
      }
      
      nanosleep(&tim_clk_half, NULL);
      GPIO_CLR = 1<<SCK;
      nanosleep(&tim_clk, NULL);
      nanosleep(&tim_clk_half, NULL);
    }

  // send mid-data ACK
  INP_GPIO(DATA);
  OUT_GPIO(DATA);
  nanosleep(&tim_clk_half, NULL);
  GPIO_CLR = 1<<DATA;
  nanosleep(&tim_r, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  INP_GPIO(DATA);
  nanosleep(&tim_r, NULL);
  GPIO_CLR = 1<<SCK;      
  nanosleep(&tim_clk, NULL);

  // read LSb
  for(i=0; i<8; i++)
    {
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk_half, NULL);
      measurement_raw<<1;
      measurement_raw += (GET_GPIO(DATA)?1:0);
      nanosleep(&tim_clk_half, NULL);
      GPIO_CLR = 1<<SCK;
      nanosleep(&tim_clk, NULL);
    }
  
  return measurement_raw;
}

float relative_humidity(uint16_t sorh, float temperature, int rh_resolution )
{
  float rh_linear, rh_true;

  if (rh_resolution == 8)
    {
      rh_linear = C1_8BIT + C2_8BIT*sorh + C3_8BIT*sorh*sorh;
      rh_true = (temperature - 25.0)*(T1_8BIT + T2_8BIT*sorh) + rh_linear;
    }
  else if (rh_resolution == 12)
    {
      rh_linear = C1_12BIT + C2_12BIT*sorh + C3_12BIT*sorh*sorh;
      rh_true = (temperature - 25.0)*(T1_12BIT + T2_12BIT*sorh) + rh_linear;
    }
  else
    {
      printf("[ERROR] Relative humidity resolution must be 8 or 12 bit.  %d was selected.\n", rh_resolution);

      return -1;
    }

  return rh_true;
}

float temperature(uint16_t sot, const char units, int temp_resolution )
{
  float temperature;
  
  if (temp_resolution == 12)
    {
      if (units == 'c' || units == 'C') {
        temperature = D1_C + D2_C_12BIT*sot; 
      } else if (units == 'f' || units == 'F'){
        temperature = D1_F + D2_F_12BIT*sot;
      } else {
        printf("[ERROR] Units must be 'C' or 'F'. '%c' was specified.", units);
        return -1;
      }
      
    }
  else if (temp_resolution == 14)
    {
      if (units == 'c' || units == 'C') {
        temperature = D1_C + D2_C_14BIT*sot;
      } else if (units == 'f' || units == 'F'){
        temperature = D1_F + D2_F_14BIT*sot;
      } else {
        printf("[ERROR] Units must be 'C' or 'F'. '%c' was specified.", units);
        return -1;
      }
    }
  else
    {
      printf("[ERROR] Temperature resolution must be 12 or 14 bit.  %d was selected.\n", temp_resolution);

      return -1;
    }
  
  return temperature;
}

float dew_point(float relative_humidity, float temperature)
{
  float dew_point;
  dew_point = TN_C_WATER * (logf(relative_humidity/100.0) + M_WATER*temperature/(TN_C_WATER + temperature))/(M_WATER - logf(relative_humidity/100.0) - M_WATER*temperature/(TN_C_WATER+temperature));

  return dew_point;
}
