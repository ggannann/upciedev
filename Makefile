#
# Makefile for creating 'upciedev' driver
# 'upciedev' is LINUX PCIe driver for MTCA devices
# Any problem concerning to functionalities of the driver
# Please contact
#		L. Petrosyan   (ludwig.petrosyan@desy.de)
#		D. Kalantaryan (davit.kalantaryan@desy.de)
#
	
MODULE_NAME = upciedev

KVERSION = $(shell uname -r)
#HOSTNAME2 = $(shell hostname)
#KO_FILES = ../../../ko_files/$(HOSTNAME2)_$(KVERSION)
KO_FILES = .
CUR_DIR=$(PWD)
MODULE_DIR = /lib/modules/$(KVERSION)/desy_mtca


$(MODULE_NAME)-objs := \
	upciedev_drv.o \
	pciedev_ufn.o \
	pciedev_probe_exp.o \
	pciedev_remove_exp.o \
	pciedev_rw_exp.o \
	pciedev_ioctl_exp.o \
	criticalregionlock.o

obj-m := $(MODULE_NAME).o 


default: compile

all: compile copy install insert

copy:
	mkdir -p $(MODULE_DIR)
	sudo cp $(KO_FILES)/$(MODULE_NAME).ko $(MODULE_DIR)/.

install:
	#-sudo rmmod pcie_gen_drv
	#sudo insmod pcie_gen_drv.ko
	#sudo cp pcie_gen_drv.ko /lib/modules/$(KVERSION)/desy_zeuthen/.

insert:
	-sudo  $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko

compinsert: compile insert

compile:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	test ! -d /lib/modules/$(KVERSION) || make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	
	
#EXTRA_CFLAGS	+= -I/usr/include
#EXTRA_CFLAGS	+= -I/doocs/develop/include
#EXTRA_CFLAGS	+= -I/doocs/develop/common/include
EXTRA_CFLAGS	+= -DUSE_SEMAPHORE



