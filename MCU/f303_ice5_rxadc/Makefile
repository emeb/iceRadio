# Makefile for STM32F30x
# 01-28-2013 E. Brombaugh

# sub directories
VPATH = .:CMSIS:StdPeriph:ff9a

# Object files
OBJECTS = 	startup_stm32f30x.o system_stm32f30x.o main.o cyclesleep.o \
			systick.o usart.o stubs.o led.o ice5.o cmd.o bitmap.o \
            adc.o shared_spi.o st7735.o ff.o diskio.o i2s.o audio.o \
            audio_lib.o iir.o rxadc.o menu.o debounce.o \
			stm32f30x_gpio.o stm32f30x_misc.o stm32f30x_rcc.o \
			stm32f30x_usart.o stm32f30x_spi.o stm32f30x_adc.o \
            stm32f30x_dma.o


# Linker script
LDSCRIPT = STM32_FLASH.ld

CFLAGS  = -g -O3 -mlittle-endian -mthumb -ffunction-sections -std=c99
CFLAGS += -Wall -Wno-strict-aliasing
CFLAGS += -I. -ICMSIS -IStdPeriph -Iff9a
CFLAGS += -DARM_MATH_CM4 -D'__FPU_PRESENT=1' -DUSE_STDPERIPH_DRIVER 
CFLAGS += -DSTM32F303xC -D'HSE_VALUE=((uint32_t)8000000)'
CFLAGS += -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16
AFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4
LFLAGS  = $(CFLAGS) -nostartfiles -T $(LDSCRIPT) -Wl,-Map=main.map
LFLAGS += -Wl,--gc-sections
#LFLAGS += --specs=nano.specs
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Executables
ARCH = arm-none-eabi
CC = $(ARCH)-gcc
CPP = $(ARCH)-g++
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb
OPENOCD = openocd
DFU = dfu-util

#CPFLAGS = --output-target=binary -j .text -j .data
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Targets
all: main.bin

clean:
	-rm -f $(OBJECTS) *.lst *.elf *.map *.dmp main.bin

flash: gdb_flash
#flash: openocd_flash
#flash: openocd_v2.1_flash
	
gdb_flash: main.elf
	$(GDB) -x flash_cmd.gdb -batch
	stty sane

openocd_flash: main.elf
	$(OPENOCD) -f openocd.cfg -c "program main.elf verify reset exit"

openocd_jlink_flash: main.elf
	$(OPENOCD) -f openocd_j-link.cfg -c "program main.elf verify reset exit"

openocd_v2.1_flash: main.elf
	$(OPENOCD) -f openocd_stlinkv2.1.cfg -c "program main.elf verify reset exit"

dfu: main.bin
	$(DFU) -a 0 -d 0483:df11 -s 0x8000000 -D main.bin

disassemble: main.elf
	$(OBJDMP) -dS main.elf > main.dis
	
dist:
	tar -c *.h *.c *.s Makefile *.cmd *.cfg openocd_doflash | gzip > minimal_hello_world.tar.gz

main.ihex: main.elf
	$(OBJCPY) --output-target=ihex main.elf main.ihex

main.bin: main.elf 
	$(OBJCPY) $(CPFLAGS) main.elf main.bin
	$(OBJDMP) $(ODFLAGS) main.elf > main.dmp
	ls -l main.elf main.bin

main.elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o main.elf $(OBJECTS) -lnosys -lm

bitmap.o: bitmap.bin
	$(OBJCPY) -I binary -O elf32-littlearm -B arm --rename-section .data=.rodata bitmap.bin bitmap.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
