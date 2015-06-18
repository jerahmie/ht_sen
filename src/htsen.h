/*
 * htsen.h
 * Humidity and temperature sensor declarations
 */

#ifndef __HT_SEN_H__
#define __HT_SEN_H__


#define SCK 22             // SHT15 SCK  = RPI GPIO 22
#define DATA 23            // SHT15 DATA = RPI GPIO 23
#define FSCK 100000        // SHT15 SCK frequency 0.1 MHz
#define TSCK 0.00001       // SCK time period (seconds)
#define TSCK_NS 20000      // SCK time period (nanoseconds)
#define TSCKH TSCK         // SCK High Time
#define TSCKL TSCK         // SCK Low Time
#define TSCKH_NS TSCK_NS   // SCK High Time (nanoseconds)
#define TSCKL_NS TSCK_NS   // SCK Low Time (nanoseconds)
#define TRISE_NS 200       // Typical rise time (nanoseconds)
#define TFALL_NS 200       // Typical fall time (nanoseconds)
#define HIGH 1             // logic high
#define LOW 0              // logic low


#define MEASURE_TEMP 3     // SHT15 measure temperature command: 00000011
#define MEASURE_REL_HUM 5  // SHT15 measure relative humidity command: 00000101
#define READ_STATUS_REG 7  // SHT15 read status register command: 00000111
#define WRITE_STATUS_REG 6 // SHT15 write status register command: 00000110

int transmission_start();
int read_status_register();
int write_status_register();
//int measure_temperature();
//int measure_humidity();
int measure_rht(uint8_t);

#endif // __HT_SEN_H__
