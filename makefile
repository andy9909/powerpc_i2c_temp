SUBDIRS=i2c dma rapidio fpga app/bmc 

uImage:
	-@$(ECHO)  "\033[41;32m  Building uImage ...   \033[0m"
	$(MAKE) -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) uImage

all:
		for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) -C $$d; done

clean:
		for d in $(SUBDIRS); do [ -d $$d ] && $(MAKE) -C $$d clean; done
			rm -f *~


.PHONY:i2c dma fpga bmc rapidio
i2c:
	$(MAKE) -C i2c
fpga:
	$(MAKE) -C fpga
bmc:
	$(MAKE) -C app/bmc
dma:
	$(MAKE) -C dma
rapidio:
	$(MAKE) -C rapidio

##################
#####INSTALL######
##################
all:install_i2c install_fpga install_ibmc install_ibmc_debug install_dma install_rapidio
install_i2c:
	    -@cp ./i2c/*.ko ${KTARGET}/lib/module/3.6.1/kernel/i2c/ -f   
install_fpga:
	    -@cp ./fpga/*.ko ${KTARGET}/lib/module/3.6.1/kernel/fpga/ -f   
install_ibmc:
	    -@cp ./app/ibmc.bin ${KTARGET}/usr/bin/ -f   
install_ibmc_debug:
	    -@cp ./app/ibmc.bin.debug ${KTARGET}/usr/bin/ -f   
install_dma:
	    -@cp ./dma/*.ko ${KTARGET}/lib/module/3.6.1/kernel/i2c/ -f   
install_rapidio:
	    -@cp ./rapidio/*.ko ${KTARGET}/lib/module/3.6.1/kernel/i2c/ -f   
