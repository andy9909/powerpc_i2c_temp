#!/bin/bash
#############
## Toolset ##
#############

if [ -z "$SDKROOT" ] ; then                                                   
    export SDKROOT=$PWD/..
fi

################################################
## SBC8548_RAPIDIO_LINUX - Hardware configuration ##
################################################

if [ "$1" = "SBC8548_RAPIDIO_LINUX" ] ; then 
echo SBC8548_RAPIDIO_LINUX Configuration selected !
export PRJECT=powerpc-module       
export PRJROOT=/home/target-project/${PRJECT}
export TARGET=powerpc-linux-gnu
export PREFIX=${PRJROOT}/tools
export TARGET_PREFIX=${PREFIX}/${TARGET}
export PATH=${PREFIX}/bin:${PATH}
export LANG=C
export ARCH=powerpc
export CROSS_COMPILE=powerpc-linux-gnu- 

#############################################
## 自定义编译宏SBC8548,用以编译不同版本平台的接口
#############################################
export CFLAGS_KERNEL=-DSBC_8548

export EXTRA_AFLAGS=-mfpu=neon
export PATH=/root/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin/:$PATH
# Pointer to linux kernel root
export KDIR=$SDKROOT/../Powerpc-linux-kernel-src
# Pointer to uboot root
export UBOOTDIR=$SDKROOT/../u-boot-powerpc-src                                    
# Pointer to linux root installation for rootfs
export KTARGET=$SDKROOT/../rootfs
# IP address to be given to the target
export LINUX_TARGETIP=192.168.1.55
# IP address of your NFS server
export LINUX_SERVERIP=192.168.1.241
# IP address of your network gateway
export LINUX_GWIP=
# Local network subnet mask
export LINUX_NETMASK=255.255.0.0
# Initial hostname for the target
export LINUX_NAME=prophet
# Kernel image
export LINUX_KERNEL=$KDIR/arch/powerpc/boot/uImage

export ECHO=echo

fi
