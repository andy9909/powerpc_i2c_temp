# Comment/uncomment the following line to disable/enable debugging
DEBUG = y
INCLUDE := $(SDKROOT)/include/
#
# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSBULL_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
EXTRA_CFLAGS += -I$(INCLUDE)
#EXTRA_CFLAGS += -I$(LCT2991_DIR)
#export EXTRA_CFLAGS
#CFLAGS += -I..

ifneq ($(KERNELRELEASE),)
# call from kernel build system
obj-m	:= rwtemp_v.o i2c_ltc2991.o
rwtemp_v-objs := LTC2991.o ThreadMain.o

else

KERNELDIR ?= $(KDIR)
PWD       := $(shell pwd)

default:
	-@$(ECHO) -e "\033[41;32m  Building i2c ...   \033[0m"
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) INCDIR=$(INCLUDE) modules ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
endif



clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions .symvers modules.order Module.symvers

depend .depend dep:
	$(CC)  $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif
