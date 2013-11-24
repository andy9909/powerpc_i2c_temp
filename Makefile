# Comment/uncomment the following line to disable/enable debugging
DEBUG = y
PLATFORM = PowerPC
#LCT2991_DIR = ./LTC2991
#>>>>>>>>>>>>>>>>>>>>>>>
ifeq ($(PLATFORM),PowerPC)   
#	LINUX_SRC = ./linux-2.6.20
CROSS_COMPILE=powerpc-linux-gnu-
#CROSS_COMPILE=arm-linux-
ARCH := powerpc          
#ARCH := arm        
#export ARCH CROSS_COMPILE        
endif       

# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSBULL_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
#EXTRA_CFLAGS += -I$(LCT2991_DIR)
#export EXTRA_CFLAGS
#CFLAGS += -I..

ifneq ($(KERNELRELEASE),)
# call from kernel build system
#obj-$(CONFIG_RWTEMP_V) += rwtemp_v.o
obj-m	:= rwtemp_v.o i2c_ltc2991.o
rwtemp_v-objs := LTC2991.o ThreadMain.o

else

KERNELDIR ?= /home/work/i2c_housir/linux-3.6.11/
PWD       := $(shell pwd)

default:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
endif



clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions .symvers

depend .depend dep:
	$(CC)  $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
