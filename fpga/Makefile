# Comment/uncomment the following line to disable/enable debugging
INCLUDE := $(SDKROOT)/include/ 
DEBUG = y
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSBULL_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I$(INCLUDE)
#CFLAGS += -I..

ifneq ($(KERNELRELEASE),)
# call from kernel build system

obj-m	:= fpga.o

else

KERNELDIR ?= $(KDIR)
PWD       := $(shell pwd)

default:
	-@$(ECHO)  "\033[41;32m  Building fpga ...   \033[0m" 
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) INCDIR=$(INCLUDE) modules ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)

endif



clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions .symvers Module.symvers modules.order

depend .depend dep:
	$(CC)  $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
