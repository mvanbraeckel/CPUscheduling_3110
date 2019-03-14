# Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
# 11/03/2019
# CIS*3110: Operating Systems A3 - CPU Scheduling: Simple Dispatcher
# --> Simulates a simple dispacter of an OS using simulated events to do CPU scheduling
 
CC = gcc
CFLAGS = -g -Wall -std=c99

all: idispatcher git

idispatcher: idispatcher.c
	$(CC) $(CFLAGS) idispatcher.c -o idispatcher

git: *.c Makefile 
	git add Makefile
	git add test_inputs
	git add test_outputs
	git add *.pdf
	git add *.c
	git commit -m "automatic backup via makefile"
	git remote rm origin
	git config credential.helper store
	git config --global credential.helper 'cache --timeout 3600'
	git remote add origin https://github.com/mvanbraeckel/CPUscheduling_3110.git
	git push -u origin master

val0:
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all <./test_inputs/test0.in

clean:
	rm -f *.o idispatcher