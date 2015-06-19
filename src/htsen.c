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

/*
 * \fn transmission_start
 * Send transmission start sequence to SHT15.
 * \return 0 if no errors.
 */
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

/*
 * \fn wait_for_device_ready
 * Poll SHT15 DATA line to determine device ready for DATA read.
 * After command is sent to the SHT15, the device will set DATA line high until
 * data is ready when DATA line is set low.
 *
 * \param poll_period_ms  DATA line polling period in milliseconds.
 * \param timeout         Timeout in milliseconds.
 * \return 0 if DATA SHT15 is ready.  max_index if function times out.
 */
int wait_for_device_ready(const float poll_period_ms, const float timeout_ms)
{
  int i;
  int max_index = (int)floor(timeout_ms/poll_period_ms);
  struct timespec tim_poll;
  const long poll_period_ns = (long)(poll_period_ms * 1.0e6);
  tim_poll.tv_sec = 0; tim_poll.tv_nsec = poll_period_ns;

  // Set DATA pin to input
  INP_GPIO(DATA);

  // Poll DATA line until pulled low or timeout
  for (i=0; i<max_index; i++)
    {
      if (GET_GPIO(DATA) == 0) {
        return 0;
      } else {
        nanosleep(&tim_poll, NULL);
      }
    }

  return max_index;
}

/*
 * \fn soft_reset
 * Send soft reset sequence to SHT15.  Clear the status register to default
 *  values.
 *
 * \return 0 if no error.
 */
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
      if (SOFT_RESET_CMD&(128>>i))
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

/*
 * \fn conn_reset
 * Reset communication with SHT15:
 * Hold DATA line hight while toggling SCK nine or more times, followed by
 * transmission start sequence
 *
 * \return 0 if no errors
 */
int conn_reset()
{
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
  for(index=0; index<9; index++)
    {
      GPIO_SET = 1<<SCK;
      nanosleep(&tim_clk, NULL);
      GPIO_CLR = 1<<SCK;
      nanosleep(&tim_clk, NULL);
    }
  transmission_start();

  return 0;
}

/*
 * \fn read_status_register
 * Read the contents of the SHT15 status register.
 *
 * \return status register contents (8-bit).
 */
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
  INP_GPIO(SCK);
  INP_GPIO(DATA);
  OUT_GPIO(SCK);
  OUT_GPIO(DATA);

  for (i=0; i<8; i++)
    {
      if (READ_STATUS_REG_CMD&(128>>i))
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
      if (GET_GPIO(DATA)) {
        status_register += 1;
      }

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

/*
 * \fn write_status_register
 * Not implememnted.
 */
int write_status_register(uint8_t status_register)
{
  int i;
  printf("write_status_register...\n");
  for (i=0; i<8; i++)
    {
      printf("%d", (WRITE_STATUS_REG_CMD&(128>>i))?1:0);
    }
  printf("\n");

  return 0;
}

/*
 * \fn measure_rht
 * Get temperature or relative humidity readout from sensor.
 * \param measurement_type 8-bit measurement command
 *
 * \return 16-bit value containing sensor readout 8-bit to 14-bit sensor
 *         data.  (Use status register to determine measurement resolution.)
 */
uint16_t measure_rht(uint8_t measurement_type)
{
  int i;
  uint16_t measurement_raw = 0;
  uint16_t sen_ack;
  struct timespec tim_r, tim_clk, tim_clk_half;
  tim_r.tv_sec = 0; tim_r.tv_nsec=TRISE_NS;      // rise/fall time
  tim_clk.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS; // clock high/low time
  tim_clk_half.tv_sec = 0; tim_clk.tv_nsec = TSCK_NS>>1; // clock high/low time

  if (measurement_type == MEASURE_TEMP_CMD)
    {
      printf("Measuring temperature...\n");
    }
  else if (measurement_type == MEASURE_REL_HUM_CMD)
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

  // poll DATA ready every 10 ms (1sec timeout)
  int data_ready_timeout = wait_for_device_ready(10.0, 1000.0);  

  if (data_ready_timeout) {
    printf("[ERROR] Device timed out waiting for SHT15 to release DATA line.");
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


float sot_to_temperature(uint16_t sot,
                  const char units,
                  uint temp_resolution)
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

float sorh_to_relative_humidity(uint16_t sorh,
                               float temperature,
                               uint rh_resolution)
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


float dew_point(float relative_humidity,
                float temperature)
{
  float dew_point;
  dew_point = TN_C_WATER * (logf(relative_humidity/100.0) +
                           M_WATER*temperature/(TN_C_WATER + temperature))/
                           (M_WATER - logf(relative_humidity/100.0) -
                           M_WATER*temperature/(TN_C_WATER + temperature));

  return dew_point;
}


/*
 * \fn get_measurements
 * High-level function to query SHT15 and convert raw data to Temperature (C),
 *  relative humidity (%), and dew point (C).
 * \param temperature_C Temperature measurement in Celcius.
 * \param relative_humidity Relative humidity percentage.
 * \param dew_point_C Dew point in Celcius.
 *
 * \return 0 if no errors.
 */
int get_measurements(float *temperature_C,
                     float *relative_humidity,
                     float *dew_point_C)
{
  uint temp_resolution, rh_resolution;
  uint16_t rot1, sorh1;
  uint8_t sht15_status;

  sht15_status = read_status_register();
  if (sht15_status&1)
    {
      temp_resolution = 12;
      rh_resolution = 8;
    }
  else
    {
      temp_resolution = 14;
      rh_resolution = 12;
    }
  rot1 = measure_rht(MEASURE_TEMP_CMD);
  sorh1 = measure_rht(MEASURE_REL_HUM_CMD);
  *temperature_C = sot_to_temperature(rot1, 'C', temp_resolution);
  *relative_humidity = sorh_to_relative_humidity(sorh1,
                                                 *temperature_C,
                                                 rh_resolution);
  *dew_point_C = dew_point(*relative_humidity, *temperature_C);

  return 0;
}
