
CROSS_COMPILE = arm-linux-gnueabihf-
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm

STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

export AS LD CC CPP AR NM
export STRIP OBJCOPY OBJDUMP

CFLAGS := -fno-builtin -Wunused-function -Wa,-mimplicit-it=thumb
CFLAGS += -I $(shell pwd)/include

# LDFLAGS := -Tlmosem.lds -lgcc -L/usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/lib/gcc/arm-linux-gnueabihf/7.5.0 -nostdlib
LDFLAGS := -Tlmosem.lds

export CFLAGS LDFLAGS

TOPDIR := $(shell pwd)
export TOPDIR

TARGET := lmosem

obj-y += hal/		\
		 lib/		\
		 kernel/	\

OBJS_PATH = $(TOPDIR)/.objs
$(shell [ -f $(OBJS_PATH) ] && (rm $(OBJS_PATH)))
$(shell touch $(OBJS_PATH))	#新建一个这样的文件，用来存储编译出来的各个目标文件的具体路径

# .ONESHELL:
all : distclean
	make -C ./ -f $(TOPDIR)/Makefile.build
	-OBJS=$$(cat $(OBJS_PATH)) && $(LD) $(LDFLAGS) -o $(TARGET).elf $${OBJS}
	$(OBJCOPY) -O binary -S -g $(TARGET).elf $(TARGET).bin
	rm $(TARGET).elf
	rm $(OBJS_PATH)
	make install

clean:
	rm -f $(shell find -name "*.o")
	rm -f $(TARGET).bin
	rm $(OBJS_PATH)

distclean:
	rm -f $(shell find -name "*.o")
	rm -f $(shell find -name "*.d")
	rm -f $(TARGET).bin
	rm $(OBJS_PATH)

install:
	./imxdownload $(TARGET).bin /dev/sdd
	# rm $(TARGET).bin load.imx
	