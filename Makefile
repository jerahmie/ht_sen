
all:
	gcc src/htsend.c src/rpi_gpio.c -Isrc/ -o hsend

gpio_test:
	gcc src/rpi_gpio.c src/htsen.c src/gpio_test.c -I/src/ -lm -o gpio_test
