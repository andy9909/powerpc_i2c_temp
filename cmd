 mount -t cifs -o username=admin //192.168.2.26/share lgh
 



CONFIG_SYS_NO_FLASH

U-BOOT ����

//LOCAL BUS дFPGA �Ĵ��� �ſ�8001��λ��ʹ��8001��������

mw.b 0xf000004f 0x10

mw.b 0xf000004f 0x0 ��λ

//ΪBOOT ����Ӳ���������޸� board Ŀ¼�����ӣ�h
make CROSS_COMPILE=powerpc-linux-gnu- ARCH=powerpc sbc8548_config

make CROSS_COMPILE=powerpc-linux-gnu- ARCH=powerpc

//copy ���ߣ��������uImageʱʹ�á�
cp /home/u-boot-2012.10/tools/mkimage /usr/sbin 

�ں˱���

 make CROSS_COMPILE=powerpc-linux-gnu- ARCH=powerpc sbc8548.dtb
 
cp arch/powerpc/configs/85xx/sbc8548_defconfig ./.config

// �����ں�

make ARCH=powerpc CROSS_COMPILE=powerpc-linux-gnu- menuconfig
//����uiamge

make ARCH=powerpc CROSS_COMPILE=powerpc-linux-gnu- uImage

���ļ�ϵͳ��
1. �����ں�Ŀ¼
make  modules CROSS_COMPILE=powerpc-linux-gnu- ARCH=powerpc

make  modules_install  ARCH=powerpc INSTALL_MOD_PATH=/home/rootfs

genext2fs -b 32768 -d rootfs ramdisk

gzip  -9  -f  ramdisk

mkimage -n 'uboot ext2 ramdisk rootfs' -A ppc -O linux -T ramdisk -C gzip -d ramdisk.gz rootfs.ext2.gz


busyBox ����

����

make CROSS_COMPILE=powerpc-linux-gnu- ARCH=powerpc

make CROSS_COMPILE=powerpc-linux-gnu- ARCH=powerpc install

elfsProg ����

mkdir build

cd build

CC=powerpc-linux-gnu-gcc ../configure --host=$TARGET --prefix=/usr --with-root-prefix="/home/rootfs" --enable-elf-shlibs --disable-evms

make 
make install
make install-libs

RAID

mdadm -C /dev/md0 -l 0 -n 4 -e 0.9 -c 4 /dev/sd[a1,b1,c1,d1]

mdadm --assemble /dev/md0 /dev/sd[a1,b1,c1,d1]

mkfs.ext2 -b 4096 /dev/sda1

1]

mkfs.ext4 -C 0x400000 /dev/md0

mkfs.ext4 -O  ^has_journal,extent,bigalloc -C 0x100000 /dev/sda1

mount /dev/md0 /mnt

mdadm -D /dev/md0
mdadm --detail /dev/md0

mount -t ext2 /dev/sda1 /mnt

mount -t ext4 /dev/sda1 /mnt

mount -t ext4 /dev/sdc1 /mnt

e2fs������� 

CC=powerpc-linux-gnu-gcc ./configure --host=$TARGET  --prefix="/home/rootfs"

make

make install_root=/home/rootfs/usr/sbin prefix="/home/rootfs/" install

make install-libs install_root=/home/rootfs/lib

U-boot ʹ��

tftp 0x1000000 u-boot.bin

erase 0xfff00000 0xffffffff

cp.b 0x1000000 0xfff00000 0x100000

/*��д����*/
tftp 0x1000000 uImage

erase 0xffb00000 0xffefffff

cp.b 0x1000000 0xffb00000 0x400000

/*��дDTB*/
erase 0xffa00000 0xffafffff

tftp 0xc00000 sbc8548.dtb

cp.b 0xc00000 0xffa00000 10240

/*��д�ļ�ϵͳ*/
erase 0xfd000000 0xfeffffff

tftp 0x2000000 rootfs.ext2.gz

cp.b 0x2000000 0xfd000000 0x1000000

save


tftp 0x2000000 fpga.bit;hostload 0x2000000;

hostload 0x2000000
setenv bootcmd 'setenv bootargs root=/dev/ram rw console=$consoledev,$baudrate $othbootargs;tftp $ramdiskaddr $ramdiskfile;tftp $loadaddr $bootfile;tftp $fdtaddr $fdtfile;bootm $loadaddr $ramdiskaddr $fdtaddr'
//����������ʽΪflash����������
/*������������������(����������ϵͳ������/tools/flash_eraseall /dev/mtd3�� /tools/flash_cp sbc8548.dtb /dev/mtd3 ��flash������д�Ͳ��������ݿ���)��Ҫ�ô����ã�*/
setenv bootcmd 'setenv bootargs root=/dev/ram rw console=$consoledev,$baudrate $othbootargs;cp.b 0xffb00000 0x1000000 0x400000;cp.b 0xffa00000 0xc00000 0x100000;cp.b 0xfd000000 0x2000000 0x1000000;mw.b 0xf000004f 0x10;bootm $loadaddr $ramdiskaddr $fdtaddr'
//����rootfs CRC����ʱ���� rootfs �ļ���С������������
setenv bootcmd 'setenv bootargs root=/dev/ram rw console=$consoledev,$baudrate $othbootargs;cp.b 0xffb00000 0x1000000 0x400000;cp.b 0xffa00000 0xc00000 0x100000;cp.b 0xfd000000 0x2000000 0x1100000;mw.b 0xf000004f 0x10;bootm $loadaddr $ramdiskaddr $fdtaddr'
//jffs2�ļ�ϵͳ����
setenv jffs2boot 'setenv bootargs root=/dev/mtdblock1 rootfstype=jffs2 rw console=$consoledev,$baudrate $othbootargs;cp.b 0xffb00000 0x1000000 0x400000;cp.b 0xffa00000 0xc00000 0x100000;mw.b 0xf000004f 0x10;bootm 1000000 - c00000'
run jffs2boot
����ֱ��
setenv bootcmd 'setenv bootargs root=/dev/mtdblock1 rootfstype=jffs2 rw console=$consoledev,$baudrate $othbootargs;cp.b 0xffb00000 0x1000000 0x400000;cp.b 0xffa00000 0xc00000 0x100000;mw.b 0xf000004f 0x10;bootm 1000000 - c00000'
/*tftp����ʹ�����ַ���*/
//����������ʽΪtftpģʽ��ȥ���������Կ���8001���ڸ�λ״̬��������8001���� mw.b 0xf000004f 0x10;
setenv bootcmd 'setenv bootargs root=/dev/ram rw console=$consoledev,$baudrate $othbootargs;mw.b 0xf000004f 0x10;tftp $ramdiskaddr $ramdiskfile;tftp $loadaddr $bootfile;tftp $fdtaddr $fdtfile;bootm $loadaddr $ramdiskaddr $fdtaddr'
����ģ��

powerpc-linux-gnu-gcc -D_FILE_OFFSET_BITS=64 testSpeed.c

NFS utils ��װ

./configure --host=$TARGET  --disable-nfsv4 --disable-gss --disable-uuid --without-tcp-wrappers --with-gnu-ld CC=powerpc-linux-gnu-gcc --prefix="/home/rootfs"

make

make DESTDIR=/home/nfs-utils install-strip


portmap ��װ

CC=powerpc-linux-gnu-gcc make

mount -o nolock 192.168.1.251:/home /mnt 

rpcbind ��װ   
 
../configure --host=$TARGET  --with-gnu-ld CC=powerpc-linux-gnu-gcc --prefix="/home/rootfs" CFLAGS=-I/home/target-project/powerpc-module/tools/include/ LDFLAGS=-L/home/target-project/powerpc-module/tools/lib 

  

libpcap ��װ

../configure --host=$TARGET CC=powerpc-linux-gnu-gcc --prefix=$PREFIX --with-pcap=linux

make 

make install  
 
NFS ��װ


nfs-utils-1.2.3 �����밲װ
--disable-tirpc

-L${exec_prefix}/lib -ltirpc

./configure --host=$TARGET --disable-nfsv4 --disable-gss  --disable-uuid --without-tcp-wrappers --with-gnu-ld CC=powerpc-linux-gnu-gcc LDFLAGS=-L/home/target-project/powerpc-module/tools/lib CFLAGS=-I/home/target-project/powerpc-module/tools/include/tirpc/

make

make install DESTDIR=/home/rootfs

portmap ��װ


make CROSS_COMPILE=powerpc-linux-gnu- ARCH=powerpc CC=powerpc-linux-gnu-gcc


rpcinfo -p 192.168.1.251

mount -t nfs -o nolock 192.168.1.251:/home/prog /tmp

tftp 0x2000000 fpga.bit;hostload 0x2000000;

mount -t ext4 /dev/sda1 /mnt

	����
	
ifconfig eth0 192.168.1.236
	
rpcinfo -p 192.168.1.236

showmount -e 192.168.1.236


mount -t nfsd nfsd /proc/fs/nfsd

portmap

exportfs -av


rpc.mountd

rpc.nfsd

rpc.statd


exportfs -o async *:/mnt


/ # exportfs -a

/ # rpc.nfsd
rpc.nfsd: unable to resolve ANYADDR:nfs to inet address: Servname not supported for ai_socktype
rpc.nfsd: unable to set any sockets for nfsd


/ # rpc.mountd

Cannot register service: RPC: Timed out

rpc.mountd

Cannot register service: RPC: Timed out

showmount -e 192.168.1.251

clnt_create: RPC: Unknown protocol


netstat -tunlp

# cat -n .config | grep SUNRPC
1555 CONFIG_SUNRPC=y
1556 CONFIG_SUNRPC_GSS=y


/ # exportfs -av
exporting *:/mnt
exportfs: /proc/fs/nfs/exports:1: unknown keyword "test-client-(rw"


CC=powerpc-linux-gnu-gcc ./configure --host=$TARGET --prefix="/usr" 



/home/nfs/nfs-utils-1.1.5/utils/exportfs/exportfs.c:399: undefined reference to `S_ISDIR'






fedora 14

rpcbind-0.2.0-7.fc14.i686

nfs-utils-1.2.3-0.fc14.i686
nfs-utils-lib-1.1.5-2.fc14.i686



CC=powerpc-linux-gnu-gcc ./configure --host=$TARGET --prefix=$PREFIX

--x-libraries=DIR

/home/target-project/powerpc-module/tools/include/tirpc/netconfig.h


--target=TYPE

./configure --host=$TARGET  CC=powerpc-linux-gnu-gcc --target=$TARGET --enable-tirpc
 
 
 LIBS=-lposix
 
 
 --prefix=PATH
 
 --bindir=PATH
 
 
 --srcdir=/home/target-project/powerpc-module/tools/include/tirpc
 
 --bindir=/home/target-project/powerpc-module/tools/include/tirpc/
 
 
 /home/target-project/powerpc-module/tools/powerpc-linux-gnu/include/sys/mount.h
 
 
 compat-glibc
 
 
 ./configure  --disable-nfsv4 --disable-gss  --disable-uuid --without-tcp-wrappers --with-gnu-ld 


[root@prophet portmap_6.0]# showmount -e 192.168.1.251
clnt_create: RPC: Program not registered



1.) use < .bashrc > file in your home directory (instead of .bash_profile)
2.) add the following line to this file :

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib ; export LD_LIBRARY_PATH

 DO ONE MORE THING : also add this line to < /etc/ld.so.conf > file
 
 
 echo ${PREFIX}/lib
 
 
 
 rpcbind: rpcbind: can't find local transport
 
 
 
 / # rpcbind
rpcbind: cannot get local address for udp: Servname not supported for ai_socktype
rpcbind: cannot get local address for tcp: Servname not supported for ai_socktype


/test # rpc.nfsd
rpc.nfsd: unable to resolve ANYADDR:nfs to inet address: Servname not supported for ai_socktype
rpc.nfsd: unable to set any sockets for nfsd



configure CFLAGS="$CFLAGS" LDFLAGS="-pie" \
    --enable-warmstarts \
    --with-statedir="$RPCBDIR" \
    --with-rpcuser="$RPCBUSR" \
    --enable-libwrap \
    --enable-debug

configure CFLAGS="$CFLAGS" --enable-warmstarts --with-statedir="$RPCBDIR" --with-rpcuser="$RPCBUSR" --enable-libwrap --enable-debug CC=powerpc-linux-gnu-gcc --host=$TARGET
configure --enable-libwrap --enable-debug CC=powerpc-linux-gnu-gcc --host=$TARGET

/home/target-project/powerpc-module/tools/include/tirpc/netconfig.h


dig myservername.thisisahmed.com
host myservername.thisisahmed.com 


powerpc-linux-gnu-readelf -a a.out > test


endservent , setservent , getservbyname , getservbyport,getservent


/test # exportfs -av
exporting *:/mnt
exportfs: /mnt does not support NFS export


/test # rpc.svcgssd
rpc.svcgssd: error while loading shared libraries: libgssapi_krb5.so.2: cannot open shared object file: No such file or directory


ext4 bigalloc ֧�� 

mkfs.ext4 -O extent,bigalloc -C 0x100000 /dev/sda1
mount -t ext4 /dev/sda1 /mnt

mount 192.168.1.236:/mnt /mnt



rpmbuild -bb --clean --target $TARGET rpcbind.spec


CFLAGS=-I/home/target-project/powerpc-module/tools/ 
LDFLAGS=-L/home/target-project/powerpc-module/tools/lib 



./configure --build=i686-linux --host=arm-linux --target=arm-linux \\
CFLAGS=-I/usr/local/arm/2.95.3/arm-linux/include \\
LDFLAGS=-L/usr/local/arm/2.95.3/arm-linux/lib \\
--prefix=/usr/local/arm/2.95.3/arm-linux \\
--enable-lite \\
--disable-galqvfb \\
--disable-qvfbial \\
--disable-vbfsupport \\
--disable-ttfsupport \\
--disable-type1support \\
--disable-imegb2312py \\
--enable-extfullgif \\
--enable-extskin \\
--disable-videoqvfb \\
--disable-videoecoslcd 
