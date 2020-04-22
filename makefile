FILENAME = main
DEVICE = ATtiny85

COMPILE = avr-g++ -Os -mmcu=$(DEVICE) -DF_CPU=8000000UL -DRMDF

default: compile upload

compile:
	$(COMPILE) -c $(FILENAME).cpp -o bin/$(FILENAME).o
	$(COMPILE) -c ds18b20/DallasTemperature.cpp -o bin/DallasTemperature.o
	$(COMPILE) -c oneWire/oneWire.cpp -o bin/oneWire.o
	$(COMPILE) -o bin/$(FILENAME).elf bin/$(FILENAME).o bin/DallasTemperature.o bin/oneWire.o
	avr-objcopy -j .text -j .data -O ihex bin/$(FILENAME).elf bin/$(FILENAME).hex

upload:
	avrdude -c usbasp -p $(DEVICE) -U flash:w:$(FILENAME).hex:i