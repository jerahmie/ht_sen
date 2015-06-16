/*
 * rpi_gpio.c
 * GPIO setup for raspberry pi.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "rpi_gpio.h"

int mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;



/*
 * Set up GPIO memory map.
 */
void setup_rpi_gpio()
{
    /* open /dev/mem */
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }

    /* mmap GPIO */
    gpio_map = mmap(
        NULL,                 // Any address space (most portable)
        BLOCK_SIZE,           // Map length
        PROT_READ|PROT_WRITE, // Enable read/write access to memory region
        MAP_SHARED,           // Shared with other processs
        mem_fd,               // File to map
        GPIO_BASE             // offset to GPIO peripheral
        );

    close(mem_fd);  // close mem_fd after mmap

    if (gpio_map == MAP_FAILED){
        printf("mmap error %d\n", (int)gpio_map);  // errno also set
        exit(-1);
    }

    // Always use a volatile pointer
    gpio = (volatile unsigned *)gpio_map;
        
} // setup_io
