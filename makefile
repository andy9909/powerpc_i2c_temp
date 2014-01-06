SUBDIRS=i2c dma rapidio fpga app/bmc 

uImage:
	-@$(ECHO)  -e "\033[41;32m  Building uImage ...   \033[0m"
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
###Purge module###
##################
.PHONY:purge_i2c purge_dma purge_fpga purge_bmc purge_rapidio
purge_i2c:
	$(MAKE) -C i2c clean
purge_fpga:
	$(MAKE) -C fpga clean
purge_bmc:
	$(MAKE) -C app/bmc clean
purge_dma:
	$(MAKE) -C dma clean
purge_rapidio:
	$(MAKE) -C rapidio clean

##################
#####INSTALL######
##################
all:install_i2c install_fpga install_ibmc install_ibmc_debug install_dma install_rapidio
install_i2c:
	    -cp ./i2c/*.ko ${KTARGET}/lib/modules/3.6.11/kernel/i2c/ -f   
install_fpga:
	    -cp ./fpga/*.ko ${KTARGET}/lib/modules/3.6.11/kernel/fpga/ -f   
install_ibmc:
	    -cp ./app/bmc/ibmc.bin ${KTARGET}/usr/bin/ -f   
install_ibmc_debug:
	    -cp ./app/bmc/ibmc.bin ${KTARGET}/usr/bin/ibmc.bin.debug -f   
install_dma:
	    -cp ./dma/*.ko ${KTARGET}/lib/modules/3.6.11/kernel/dma/ -f   
install_rapidio:
	    -cp ./rapidio/*.ko ${KTARGET}/lib/modules/3.6.11/kernel/rapidio/ -f   
