/*
 * htsen.c
 * Humidity and temperature sensor interface.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "rpi_gpio.h"
#include "htsen.h"

// I/O access
volatile unsigned *gpio;


int transmission_start()
{
  printf("Starting send command.\n");
  
  struct timespec tim1;
  tim1.tv_sec = 0; tim1.tv_nsec = TSCK_NS;
  
  INP_GPIO(SCK);
  INP_GPIO(DATA);
  OUT_GPIO(SCK);
  OUT_GPIO(DATA);

  GPIO_CLR = 1<<SCK;
  GPIO_SET = 1<<DATA;
  nanosleep(&tim1, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim1, NULL);
  GPIO_CLR = 1<<DATA;
  nanosleep(&tim1, NULL);
  GPIO_CLR = 1<<SCK;
  nanosleep(&tim1, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim1, NULL);
  GPIO_SET = 1<<DATA;
  nanosleep(&tim1, NULL);
  GPIO_CLR = 1<<SCK;
  nanosleep(&tim1, NULL);
  GPIO_CLR = 1<<DATA;
  return 0;
}

int read_status_register()
{
  int i;
  printf("read_status_register...\n");
  for (i=0; i<8; i++)
    {
      printf("%d", (READ_STATUS_REG&(128>>i))?1:0);
    }
  printf("\n");
  
  return 0;
}

int write_status_register()
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

int measure_rht( uint8_t measurement_type)
{
  int i;
  uint16_t measurement_raw = 0;
  struct timespec tim_r, tim_clk, tim_clk_half;
  tim_r.tv_sec = 0; tim_r.tv_nsec=TRISE_NS;      // rise/fall time
  tim_clk.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS; // clock high/low time
  tim_clk_half.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS/2; // clock high/low time
  if ( measurement_type == MEASURE_TEMP )
    {
      printf("Measuring temperature...\n");
    }
  else if ( measurement_type == MEASURE_REL_HUM )
    {
      printf("Measuring humidity...\n");
    }
  
  INP_GPIO(SCK);
  INP_GPIO(DATA);
  OUT_GPIO(SCK);
  OUT_GPIO(DATA);
  
  for (i=0; i<8; i++)
    {
	nanosleep(&tim_r, NULL);
	if (measurement_type&(128>>i)) {
	    GPIO_SET = 1<<DATA;
	} else {
	    GPIO_CLR = 1<<DATA;
	}
	GPIO_SET = 1<<SCK;
	nanosleep(&tim_clk, NULL);
	GPIO_CLR = 1<<SCK;
//	nanosleep(&tim_clk, NULL);
    }
  INP_GPIO(DATA);
  nanosleep(&tim_clk, NULL);
  GPIO_SET = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  int sen_ack = GET_GPIO(DATA);
  nanosleep(&tim_clk, NULL);
  GPIO_CLR = 1<<SCK;
  nanosleep(&tim_clk, NULL);
  if (sen_ack != 0 )
    {
      printf("[ERROR] Did not receive ACK from sensor! Exiting.\n");
      return 0;
    }
  else 
    {
      // read MSb
      for(i=0; i<8; i++)
        {
          GPIO_SET = 1<<SCK;
          nanosleep(&tim_clk, NULL);
          measurement_raw << 1;
          measurement_raw | GET_GPIO(DATA);
          nanosleep(&tim_clk, NULL);
          GPIO_SET = 0<<SCK;
          nanosleep(&tim_clk, NULL);
        }
      // mid-data ACK
      INP_GPIO(DATA);
      OUT_GPIO(DATA);
      GPIO_CLR = 1<<DATA;
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk, NULL);
      INP_GPIO(DATA);
      GPIO_CLR = 1<<SCK;
      nanosleep(&tim_clk, NULL);
      // read LSb
      for(i=0; i<8; i++)
        {
          GPIO_SET = 1<<SCK;
          nanosleep(&tim_clk_half, NULL);
          measurement_raw << 1;
          measurement_raw | GET_GPIO(DATA);
          nanosleep(&tim_clk_half, NULL);
          GPIO_CLR = 1<<SCK;
          nanosleep(&tim_clk, NULL);
        }
    }
  
  return (int) measurement_raw;
}
