obj-m += thermal_controller.o

KERNEL_DIR ?= /home/alok/wsl-kernel-6.6.87.2
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
