
INCLUDE:=-I../../include

CFLAGS:=$(INCLUDE) 
CC=powerpc-linux-gnu-gcc
ibmc.bin:ibmc.o
	-@$(ECHO) -e "\033[41;32m  Building bmc ...   \033[0m" 
	$(CC) $< -o $@ 
sensor_read_userspace_test:sensor_read_userspace_test.o
	$(CC) $< -o $@ 

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) 
.PHONY:clean
clean:
	-@$(ECHO) -e "\033[41;32m  Cleaning bmc ...   \033[0m" 
	-$(RM) *.o *.out iBmc sensor_read_userspace_test *.bin
