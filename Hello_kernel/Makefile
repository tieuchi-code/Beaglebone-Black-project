obj-m := hello-kernel.o 
KERNELDIR=/home/tieuchi/ti-processor-sdk-linux-am335x-evm-09.03.05.02/board-support/ti-linux-kernel-6.1.119+gitAUTOINC+c490f4c0fe-ti
CURRENTDIR=$(shell pwd)
CROSS=/home/tieuchi/ti-processor-sdk-linux-am335x-evm-09.03.05.02/external-toolchain-dir/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-

all:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENTDIR) ARCH=arm CROSS_COMPILE=$(CROSS) modules
install:
	scp hello-kernel.ko root@192.168.8.8:/home/root
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENTDIR) ARCH=arm CROSS_COMPILE=$(CROSS) clean
help:
	$(MAKE) -C $(KERNELDIR) help
