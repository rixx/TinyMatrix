OUTPUT=firmware
OBJECTS=src/main.o											\


CC=avr-gcc
LD=avr-ld
OBJCOPY=avr-objcopy

CPUSPEED=4000000
MCU=attiny2313
CPUTYPE=t2313
CFLAGS=-mmcu=$(MCU) -I. -Os -DF_CPU=$(CPUSPEED)
AVRDUDE=avrdude


all: $(OBJECTS)
	$(CC) -Os -Wl,--gc-sections -mmcu=$(MCU) -o $(OUTPUT).elf $(OBJECTS) -lm
	$(OBJCOPY) -j .text -j .data -O ihex $(OUTPUT).elf $(OUTPUT).elf.ihex

flash:
	$(AVRDUDE) -p$(CPUTYPE) -cstk500v2 -P/dev/ttyACM3 -Uflash:w:$(OUTPUT).elf.ihex:i -U hfuse:w:0xDF:m -U lfuse:w:0xE2:m -B 10

clean:
	rm $(OBJECTS) $(OUTPUT).elf $(OUTPUT).elf.ihex -f


