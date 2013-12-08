#!/bin/bash
#将当前的文件备份，并copy新的文件过来
#运行前请先载入环境变量 . ./setenv.sh
srcfile_list="fsldma.c fsldma.h dmatest.c"
for src_file in ${srcfile_list}
do
    mv ${src_file} ./${src_file}.back -f
    cp ${KDIR}/drivers/dma/${src_file} ./ -f
    if [ $? -ne 0 ];then
        echo "cp ${src_file} fail "
        exit -1
    else
        echo "cp ${KDIR}/drivers/dma/${src_file} ./ -f"
    fi
done

src_file="dma.c"

mv ${src_file} ./${src_file}.back -f
cp ${KDIR}/arch/powerpc/kernel/${src_file} ./ -f
if [ $? -ne 0 ]; then
    echo "cp ${src_file} fail "
    exit -1
else
    echo "cp ${KDIR}/arch/powerpc/kernel/${src_file} ./ -f"
fi
