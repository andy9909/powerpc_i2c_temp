#/bin/sh
genext2fs -N 4096 -b 65536 -d rootfs ramdisk
gzip  -9  -f  ramdisk
mkimage -n 'uboot ext2 ramdisk rootfs' -A ppc -O linux -T ramdisk -C gzip -d ramdisk.gz rootfs.ext2.gz
cp rootfs.ext2.gz /home/lgh
