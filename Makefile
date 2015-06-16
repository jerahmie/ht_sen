
all:
	gcc src/htsend.c src/rpi_gpio.c -Isrc/ -o hsend
