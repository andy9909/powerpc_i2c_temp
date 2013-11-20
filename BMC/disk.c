#include <sys/statfs.h>  
#include <stdio.h>  
int main()  
{  
    struct statfs diskInfo;  
    while(1)
{  
    statfs("/home", &diskInfo);  
    unsigned long long blocksize = diskInfo.f_bsize;    //..block.......  
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;   //......f_blocks.block...  
    printf("blocksize=%llu, f_blocks=%lu,free=%lu\n",blocksize,diskInfo.f_blocks,diskInfo.f_bfree);
    printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",   
        totalsize, totalsize>>10, totalsize>>20, totalsize>>30);  
      
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize; //.......  
    unsigned long long availableDisk = diskInfo.f_bavail * blocksize;   //......  
    printf("Disk_free = %llu MB = %llu GB\nDisk_available = %llu MB = %llu GB\n",   
        freeDisk>>20, freeDisk>>30, availableDisk>>20, availableDisk>>30);  
    sleep(5);
 }     
    return 0;  
} 
