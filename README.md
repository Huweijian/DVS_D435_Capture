# DVS_D435_Capture

## 依赖 
[librealsense build 2.34.0](https://github.com/IntelRealSense/librealsense/blob/master/doc/distribution_linux.md)   
[RealSense D435 Firmware 5.12.3](https://dev.intelrealsense.com/docs/firmware-releases)   
SEES C++ SDK v1.5.1   

## 保存格式 
保存在"Capture-时间戳"文件夹中

### D435 
保存红外图像，时间戳，曝光时间 
### DVS 
 - 以二进制的形式保存事件，每13个Bytes为一个事件，定义如下： 
```
struct BinPacket
{
    uint64_t ts;// us
    uint16_t x;
    uint16_t y;
    unsigned char polar; 
};
```
 - 每隔100ms保存一张事件的累积图
