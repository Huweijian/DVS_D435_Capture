# DVS_D435_Capture
## D435 
保存红外图像，时间戳，曝光时间 
## DVS 
 - 以二进制的形式保存事件，每13个Bytes为一个事件，定义如下： 
```
struct BinPacket
{
    uint64_t ts;
    uint16_t x;
    uint16_t y;
    unsigned char polar;
};
```
 - 每隔100ms保存一张事件的累积图
