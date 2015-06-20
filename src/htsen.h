/*
 * htsen.h
 * Humidity and temperature sensor declarations
 */

#ifndef __HT_SEN_H__
#define __HT_SEN_H__


#define SCK 22             // SHT15 SCK  = RPI GPIO 22
#define DATA 23            // SHT15 DATA = RPI GPIO 23
//#define FSCK 100000        // SHT15 SCK frequency 0.1 MHz
//#define TSCK 0.00001       // SCK time period (seconds)
#define TSCK_NS 10000      // SCK time period (nanoseconds)
#define TSCKH TSCK         // SCK High Time
#define TSCKL TSCK         // SCK Low Time
#define TSCKH_NS TSCK_NS   // SCK High Time (nanoseconds)
#define TSCKL_NS TSCK_NS   // SCK Low Time (nanoseconds)
#define TRISE_NS 200       // Typical rise time (nanoseconds)
#define TFALL_NS 200       // Typical fall time (nanoseconds)
#define HIGH 1             // logic high
#define LOW 0              // logic low


#define MEASURE_TEMP_CMD 3     // SHT15 measure temperature command: 00000011
#define MEASURE_REL_HUM_CMD 5  // SHT15  relative humidity command: 00000101
#define READ_STATUS_REG_CMD 7  // SHT15 read status register command: 00000111
#define WRITE_STATUS_REG_CMD 6 // SHT15 write status register command: 00000110
#define SOFT_RESET_CMD 30      // SHT15 soft reset command: 00011110

// Relative humidity
#define C1_8BIT -2.0468
#define C2_8BIT 0.5872
#define C3_8BIT -0.00040845
#define C1_12BIT -2.0468
#define C2_12BIT 0.0367
#define C3_12BIT -0.0000015966

// Temperature conversion coefficients 3.3V
#define D1_C -39.65
#define D2_C_12BIT 0.04
#define D2_C_14BIT 0.01
#define D1_F -39.4
#define D2_F_12BIT 0.072
#define D2_F_14BIT 0.018

// Temperature Humidity compensation coefficients
#define T1_8BIT 0.01
#define T2_8BIT 0.00128
#define T1_12BIT 0.01
#define T2_12BIT 0.00008

// Dew point coefficients 
#define TN_C_WATER 243.12
#define M_WATER 17.62
#define TN_C_ICE 272.62
#define M_ICE 22.46

int transmission_start();
int wait_for_device_ready(float, float);
int soft_reset();
int conn_reset();
uint8_t read_status_register();
int write_status_register(uint8_t);
uint16_t measure_rht(uint8_t);
float sot_to_temperature(uint16_t, const char, uint);
float sorh_to_relative_humidity(uint16_t, float, uint);
float dew_point(float, float);
int get_measurements(float *, float*, float* );

#endif // __HT_SEN_H__
