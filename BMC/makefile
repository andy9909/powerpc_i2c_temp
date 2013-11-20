
CC=powerpc-linux-gcc
iBmc:iBmc.o
	$(CC) $< -o $@ 
%.o:%.c
	$(CC) -c $< -o $@

.PHONY:clean
clean:
	-$(RM) *.o *.out
