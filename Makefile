
all:
	gcc src/rpi_gpio.c  src/htsen.c src/htsend.c -Isrc/ -lm -o htsend

gpio_test:
	gcc src/rpi_gpio.c src/htsen.c src/gpio_test.c -I/src/ -lm -o gpio_test
