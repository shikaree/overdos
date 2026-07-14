######################################################################
# Configuration							     #
#								     #
# Defineable options are:					     #
#								     #
# MAX_PROCESSES - Default 64.					     #
# STACK_SIZE	- Default 65536.				     #
# MAX_MBOX	- Default 192.					     #
# MAX_MSG	- Default 8 (is number of msgs/mailbox).	     #
#								     #
# ALL of these MUST be defined.					     #
######################################################################

CONFIG = -DMAX_PROCESSES=64 -DSTACK_SIZE=65536 -DMAX_MBOX=192 -DMAX_MSG=8

######################################################################
# Main 								     #
######################################################################
CC = /usr/sup/sde/bin/mgcc
AR = /usr/sup/sde/bin/mar
OPTIMIZATION = -fomit-frame-pointer -finline-functions -fthread-jumps -fdelayed-branch
CFLAGS = -Wall -EB -mcpu=r4k -mips2 -Ttext 80020000  -ansi -Iinclude -nostdlib -nostartfiles -msoft-float -G0 -DLOCALE_SE $(OPTIMIZATION) $(CONFIG)
ARFLAGS = rvs
OBJECTS = $(patsubst %.c,%.o,$(wildcard kernel/*.c)) $(patsubst %.c,%.o,$(wildcard drivers/*.c)) $(patsubst %.sx,%.o,$(wildcard kernel/*.sx)) $(patsubst %.sx,%.o,$(wildcard drivers/*.sx)) $(patsubst %.c,%.o,$(wildcard apps/*.c)) $(patsubst %.sx,%.o,$(wildcard apps/*.sx))
OBJECTS_LIB =  $(patsubst %.c,%.o,$(wildcard lib/*.c)) $(patsubst %.sx,%.o,$(wildcard lib/*.sx)) 
DATE = $(shell date +%y%m%d)
PWD = $(shell pwd)
DISTDIR = $(notdir $(PWD))
PACKER = bzip2 -f -9

KERNEL = bin/bootfile
LIBRARY = bin/libos.a

all: bin/bootfile

bin/bootfile : bin/libos.a $(patsubst %.o, objects/%.o,$(OBJECTS)) 
	$(CC) $(CFLAGS) $(patsubst %.o, objects/%.o,$(OBJECTS)) $(LIBRARY) -o $(KERNEL)

dist: clean_dist 	
	cd .. ; tar -c $(DISTDIR) > commit-$(DATE).tar	
	$(PACKER) ../commit-$(DATE).tar

bin/libos.a: $(patsubst %.o, objects/%.o,$(OBJECTS_LIB))
	$(AR) $(ARFLAGS) $(LIBRARY) $(patsubst %.o, objects/%.o,$(OBJECTS_LIB)) 
        
objects/%.o : %.sx
	$(CC) $(CFLAGS) -c $< -o objects/$(patsubst %.sx,%.o,$<)

objects/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o objects/$(patsubst %.c,%.o,$<)

clean:
	rm -f objects/kernel/*.o objects/drivers/*.o objects/apps/*.o
	rm -f bin/bootfile

clean_dist: clean
	rm -f objects/lib/*.o 
	rm -f bin/libos.a
	rm -f lib/*.bak apps/*.bak kernel/*.bak drivers/*.bak include/*.bak doc/*.bak
	rm -f lib/*.*~ apps/*.*~ kernel/*.*~ drivers/*.*~ include/*.*~ doc/*.*~
	rm -f lib/#*.* apps/#*.* kernel/#*.* drivers/#*.* include/#*.* doc/#*.*
	rm -f lib/*.org apps/*.org kernel/*.org drivers/*.org include/*.org doc/*.org
	chmod 775 lib apps kernel drivers include doc bin objects objects/*		
	chmod 664 lib/* apps/* kernel/* drivers/* include/* doc/* Makefile

