# Comment/uncomment the following line to disable/enable debugging
INCLUDE: = ../include
DEBUG = y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSBULL_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
#CFLAGS += -I..

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= fpga.o

else

KERNELDIR ?= /home/work/NFS/vpx3-ssd1/linux-3.6.11/
PWD       := $(shell pwd)

default:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) INCDIR=$(INCLUDE) modules ARCH=powerpc CROSS_COMPILE=powerpc-linux-gnu-

endif



clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

depend .depend dep:
	$(CC)  $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
