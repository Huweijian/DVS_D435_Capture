# DVS_D435_Capture

## 运行环境
[ros](http://wiki.ros.org/)
[librealsense build 2.34.0](https://github.com/IntelRealSense/librealsense/blob/master/doc/distribution_linux.md)   
[RealSense D435 Firmware 5.12.3](https://dev.intelrealsense.com/docs/firmware-releases)   
SEES C++ SDK v1.5.1，相见组内share  

## 数据保存 
保存在"Capture-时间戳"文件夹中

### D435 
保存红外图像，时间戳，曝光时间 
### DVS 
以rosbag形式保存iniVation DVS相机的强度图，事件和IMU数据，事件的保存类型是uzh的[dvs_msgs](https://github.com/uzh-rpg/rpg_dvs_ros/tree/master/dvs_msgs)