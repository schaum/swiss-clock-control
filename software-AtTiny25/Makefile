DEVICE     = attiny25
CLOCK      = 3686400
PROGRAMMER = -c usbasp
OBJECTS    = main.o
FUSES      = -U hfuse:w:0xdf:m -U lfuse:w:0x7d:m

# ...
# ATMega25 fuse bits (fuse bits for other devices are different!):
# Example for 8 MHz internal oscillator: see main.c for clocking description
# Fuse high byte:
# 0xdf = 1 1 0 1   1 1 1 1
#        ^ ^ ^ ^   ^ \-+-/
#        | | | |   |   +------ BODLEVEL 2..0
#        | | | |   +---------- EESAVE (set to 0 to preserve EEPROM over chip erase)
#        | | | +-------------- WDTON (if set to 0, watchdog is always on)
#        | | +---------------- SPIEN (if set to 1, serial programming is disabled)
#        | +------------------ DWEN (Debug wire enable: off if 1)
#        +-------------------- RSTDISBL (if set to 0, RESET pin is disabled)
# Fuse low byte:
# 0x7d = 0 1 1 1   1 1 0 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (3.6864 Mhz external crystal)
#        | |  +--------------- SUT 1..0 (slowly rising power)
#        | +------------------ CKOUT (clock out enabled if 0)
#        +-------------------- CKDIV8 (clock dividet by 8 if 0)
# Fuse extended byte:
# 0xff = 1 1 1 1   1 1 1 1 <-- SELFPRGEN (self-programming enabled if 0, default is 1)
#        \------+------/
#               +------------- NOT USED (all set to 1, means unprogrammed)

# Tune the lines below only if you know what you are doing:

SIZE = avr-size --mcu=$(DEVICE) -C main.elf

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@


flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

size:
	avr-nm --size-sort -S main.elf
	$(SIZE)

main.map:
	$(COMPILE) -Wl,-Map,main.map $(OBJECTS)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	$(SIZE)
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(COMPILE) -E main.c
