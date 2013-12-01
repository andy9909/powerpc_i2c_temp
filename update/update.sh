#!/bin/sh
#自动升级根文件系统 内核镜像 和 设备目录树文件的 的脚本 通过
#通过/proc/versionn中的信息  利用/tool目录中的烧写工具对flasd进行烧写 (/dev/mtd2)
filepath="/home/work/NFS/update/"
toolspath="/tools/"
#filepath="/mnt/version-release/test"
path=$(pwd)
uImage="$path/uImage"
dtb="/$path/sbc8548.dtb"
rootfs="/$path/rootfs.ext2.gz"
path_rootfs_ver="/version/version"
echo -e "\033[41;36m Warning: update will take up several minutes,please keep patience ......do not plug out power!!!!!! \033[0m "
sudo chmod 777 $(pwd)
tar zxvf vpx3-ssd1*
#check file path Ok
if [ ! -d "$path" ];
then
echo "file path not exist... update fail!!!exit"
exit
fi
#check kernel image version
echo "check kernel image version......"
cat /proc/version  >/tmp/test.txt
sleep 1
cat /tmp/test.txt |while read line
do 
kernel_version=`awk 'BEGIN {split("'"$line"'",arr);print arr[3]}'`
echo -e "\033[40;36mkernel_version:\033[0m $kernel_version"
cat version |while read line
do
new_kernel_version=`awk 'BEGIN {split("'"$line"'",arr);print arr[2]}'`
echo -e "\033[40;36mnew_kernel_version:\033[0m $new_kernel_version"
if [ $kernel_version != $new_kernel_version ];
then
echo "version $kernel_version was old ....will update!"
image_update_flag="TRUE"
#check uImage file exist or not
if [ ! -f "$uImage" ];
then
echo -e "\033[41;36m update file uImage not exist,please put  file uImage in Path $filepath \033[0m"
exit
else
echo "file uImage check Ok"
fi
#update uImage
/$toolspath/flash_eraseall /dev/mtd3
echo "eraseall image partition success"
/$toolspath/flashcp uImage /dev/mtd3
echo "update kernel image success"
rm /tmp/test.txt
else
echo "version $kernel_version is the latest..... skip update!"
image_update_flag="FALSE"
rm /tmp/test.txt
fi
done
done
#check rootfs_path_version
if [  ! -f "$path_rootfs_ver" ] ;
then
mkdir /version
touch /version/version
echo "original version" > $path_rootfs_ver
fi
#check rootfs_version
cat  /version/version |while read line
do
rootfs_version=`awk 'BEGIN {split("'"$line"'",arr);print arr[2]}'`
echo -e "\033[40;36mrootfs_version:\033[0m $rootfs_version"
sleep 1
cat version |while read line
do
new_rootfs_version=`awk 'BEGIN {split("'"$line"'",arr);print arr[3]}'`
echo -e "\033[40;36mnew_rootfs_version:\033[0m $new_rootfs_version"
if [ $rootfs_version != $new_rootfs_version ];
then
echo "version $rootfs_version was old ....will update!"
rootfs_update_flag="TRUE"
#check rootfs file exist or not
if [ ! -f "$rootfs" ];
then
echo -e "\033[41;36m update file rootfs not exist, please put file rootfs.ext2.gz in path $filepath \033[0m"
exit
else
echo "file rootfs check ok"
fi
/$toolspath/flash_eraseall /dev/mtd1
echo "earse partition rootfs success"
/$toolspath/flashcp rootfs.ext2.gz /dev/mtd1
echo "update file system success"
else
echo "version $rootfs_version is the latest..... skip update!"
rootfs_update_flag="FALSE"
fi
done
done
#check dtb file exist or not
if [ ! -f "$dtb" ];
then
echo -e "\033[41;36m update file sbc8548.dtb not exist,please put file sbc8548.dtb in Path $filepath \033[0m"
exit
else
echo "file sbc8548.dtb check ok"
#update DTB
/$toolspath/flash_eraseall /dev/mtd2
echo "earse partition dtb success"
/$toolspath/flashcp sbc8548.dtb /dev/mtd2
echo "update configre file success"
fi
sleep 1
#rm -rf sbc8548.dtb uImage rootfs* version
rm -rf /tmp/*
echo "update done"
