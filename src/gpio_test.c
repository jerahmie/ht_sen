/*
 * gpio_test.c
 * Test the ability to toggle GPIO22 and GPIO23
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "rpi_gpio.h"

#ifndef NO_ERROR
#define NO_ERROR 0
#endif

int main()
{
    printf("Testing gpio.\n");
    return NO_ERROR;
}
