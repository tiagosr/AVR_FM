#################################
# Makefile for AVR_FM
# (c) 2011 Tiago Rezende

################ project files and definitions
PROGNAME = avr_fm
MCU_TARGET = atmega48
PROGRAMMER_MCU = m48

# put other files in this line
PROJSRC = main.c fm_synth.c

LIBS = 
INC =

################ generated files definition

MAPFILE = $(PROGNAME).map
TARGET = $(PROGNAME).out
DUMPTARGET = $(PROGNAME).s
HEXROMTARGET = $(PROGNAME).hex
HEXTARGET = $(HEXROMTARGET) $(PROGNAME).ee.hex
GDBINITFILE = gdbinit-$(PROGNAME)

################ Program options section
AVR_TOOLS_PREFIX=/usr/local/Crosspack-AVR
CC = $(AVR_TOOLS_PREFIX)/bin/avr-gcc
OBJCOPY = $(AVR_TOOLS_PREFIX)/bin/avr-objcopy
OBJDUMP = $(AVR_TOOLS_PREFIX)/bin/avr-objdump
SIZE = $(AVR_TOOLS_PREFIX)/bin/avr-size
AVRDUDE = $(AVR_TOOLS_PREFIX)/bin/avrdude
REMOVE = rm -f
CFLAGS = -I. $(INC) -Wall -Os -mmcu=$(MCU_TARGET) \
         -fpack-struct -fshort-enums -funsigned-bitfields \
		 -Wa,-ahlms=$(firstword $(filter %.lst, $(<:.c=.lst)))
ASMFLAGS = -I. $(INC) -mmcu=$(MCU_TARGET) \
           -Wa,-gstabs,-ahlms=$(firstword $(filter %.lst, $(<:.S=.lst)))
LDFLAGS = -Wl,-Map,$(MAPFILE) -mmcu=$(MCU_TARGET) $(LIBS)

HEXFORMAT = ihex

AVRDUDE_PROGRAMMER_ID = USBtinyISP

################ Filters section
CFILES = $(filter %.c, $(PROJSRC))
ASMFILES = $(filter %.S, $(PROJSRC))

OBJDEPS=$(CFILES:.c=.o) $(ASMFILES:.S=.o)

LST=$(filter %.lst, $(OBJDEPS:.o=.lst))

GENASMFILES = $(filter %s, $(OBJDEPS:.o=.s))

.SUFFIXES : .c .S .o .out .s .hex .ee.hex .h

.PHONY : writeflash clean stats gdbinit stats

################ Targets section
all: $(TARGET)

hex: $(HEXTARGET)

writeflash:

# generating the target
$(TARGET): $(OBJDEPS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJDEPS)

# asm from C
%.s: %.c
	$(CC) -S $(CFLAGS) $< -o $@

# asm from hand-coded asm
%.s: %.S
	$(CC) -S $(ASMFLAGS) %< -o $@

## generating object files
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.S.o:
	$(CC) $(ASMFLAGS) -c $< -o $@

## generating hex files
.out.hex:
	$(OBJCOPY) -j .text -j.data -O $(HEXFORMAT) $< $@

.out.ee.hex:
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 \
			-O $(HEXFORMAT) $< $@

## generating gdb init file
gdbinit: $(GDBINITFILE)

$(GDBINITFILE): $(TARGET)
	@echo "file $(TRG)" > $(GDBINITFILE)
	@echo "target remote localhost:1212" >> $(GDBINITFILE)
	@echo "load" >> $(GDBINITFILE)
	@echo "break main" >> $(GDBINITFILE)
	@echo "continue" >> $(GDBINITFILE)
	@echo
	@echo "Use 'avr-gdb -x $(GDBINITFILE)'"

## cleanup
clean:
	$(REMOVE) $(TARGET) $(MAPFILE) $(DUMPTARGET)
	$(REMOVE) $(OBJDEPS)
	$(REMOVE) $(LST) $(GDBINITFILE)
	$(REMOVE) $(GENASMFILES)
	$(REMOVE) $(HEXTARGET)
