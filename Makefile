obj-m += syslink_drv.o

# For GXV3175v2
LINUXKERNEL_INSTALL_DIR = /root/Project/gxv3170v2/linux-2.6.32-psp/
CROSS_COMPILE = /usr/local/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi- 

# For GXV3140
#LINUXKERNEL_INSTALL_DIR = /root/Project/linux_2_6_10/
#CROSS_COMPILE =/opt/uclibc-toolchain/gcc-4.1.2/toolchain-armv5t/bin/arm-linux-

all:
	make -C $(LINUXKERNEL_INSTALL_DIR) CROSS_COMPILE=$(CROSS_COMPILE) \
		M=$(PWD) modules
clean:
	make -C $(LINUXKERNEL_INSTALL_DIR) CROSS_COMPILE=$(CROSS_COMPILE) \
		M=$(PWD) clean
