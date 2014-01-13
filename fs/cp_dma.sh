#!/bin/bash
#将当前的文件备份，并copy新的文件过来
#运行前请先载入环境变量 . ./setenv.sh

#src_file="dma.c"

#mv *.[ch] ./${src_file}.back -f
cp ${KDIR}/fs/nfsd/*.[ch] ./nfsd/ -rf
if [ $? -ne 0 ]; then
    echo "cp *.[ch] fail "
    exit -1
else
    echo "cp ${KDIR}/fs/nfsd/*.[ch] ./ -f"
fi
