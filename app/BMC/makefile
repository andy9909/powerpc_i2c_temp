
MACRO :=-DDBG__USER
INCLUDE:=-I../../include

CFLAGS:=$(INCLUDE) $(MACRO)
CC=powerpc-linux-gnu-gcc
ibmc.bin:ibmc.o
	$(CC) $< -o $@ 
sensor_read_userspace_test:sensor_read_userspace_test.o
	$(CC) $< -o $@ 

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) 
.PHONY:clean
clean:
	-$(RM) *.o *.out iBmc sensor_read_userspace_test *.bin
