rioMapOutb 0xa0000000,0x40000000,0x100000,0x14,0

d 0xa0000000
*0xa0000000 = 0x1234

rioMapInb 0x18000000,0x40000000,0x100000,0


d 0x18000000

//dma暂时不做。

//doorbell
1: 不需要设置in/out ，直接收中断

//message
VX rioSendMsg 0,0 ,0x14,0x18000000,8

rioSysInit
rioSystemDisplay

rioSendDoorbell 0,0x14,0x1122





mw.l 0xe00c0060 0xffffff


echo "sendmsg 0x12 0 0x18000000 0x10">/dev/wan_major0


echo "getmsg 0x12 0 ">/dev/wan_major0



