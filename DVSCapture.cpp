#include <iness_common/device/sees/sees.hpp>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <signal.h>
#include <ostream>
#include <thread>
#include <chrono>
#include <sys/stat.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <rosbag/bag.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Image.h>
#include <dvs_msgs/EventArray.h>
#include <cv_bridge/cv_bridge.h>

rosbag::Bag bag;
int dvs_height;
int dvs_width;
const unsigned int DVS_START_CAP = 1e6; // 1 sec

void ImuPacketCallback(iness::Imu6EventPacket &_packet)
{
    static int last_sec = -1;
    static unsigned int imu_seq = 0;
    // How to iterate through the event packet and retrieve the timestamp of the events.
    for(auto& event : _packet)
    {
        // Get the timestamp of the event.
        iness::time::TimeUs ts = event.getTimestampUs(_packet.header().event_ts_overflow);
        if(ts < DVS_START_CAP) continue;
        sensor_msgs::Imu imu;
        imu.header.seq = imu_seq++;
        imu.header.stamp = ros::Time(ts/1e6);
        imu.linear_acceleration.x = event.getAccelerationX();
        imu.linear_acceleration.y = event.getAccelerationY();
        imu.linear_acceleration.z = event.getAccelerationZ();
        imu.angular_velocity.x = event.getGyroX();
        imu.angular_velocity.y = event.getGyroY();
        imu.angular_velocity.z = event.getGyroZ();
        bag.write("/dvs/imu",imu.header.stamp, imu);

        if(ts / 1000000 != last_sec){
            last_sec = ts / 1000000;
            printf("Capture imu %d second\n", last_sec);
        }
    }
}


void FrameCallback(iness::FrameEventPacket &_packet)
{
    using namespace cv;
    static unsigned int frame_seq = 0;

    for(auto& frm : _packet)
    {
        iness::time::TimeUs ts = frm.getTimestampUs(_packet.header().event_ts_overflow);
        if(ts < DVS_START_CAP) continue;
        Mat img = frm.getImage();
        if(img.empty()) {
            printf(" * WARNING! empty frame\n");
            continue;
        }
        imshow("img", img); 
        if(waitKey(1) == 'q'){
            bag.close();
            exit(0);
        }

        if(img.type() != CV_16UC1){
            std::cout << "image is not 16UC1 !" << img.type() << std::endl;
        }

        std_msgs::Header hd;
        hd.seq = frame_seq++;
        hd.stamp = ros::Time(ts / 1e6);
        
        cv_bridge::CvImage img_tmp(hd, "mono16", img);
        sensor_msgs::Image img_msg;
        img_tmp.toImageMsg(img_msg);
        bag.write("/dvs/image_raw", hd.stamp, img_msg);
    }
}

void polarityEventPacketCallback(iness::PolarityEventPacket &_packet)
{
    static unsigned int evt_seq = 0;

    iness::time::TimeUs ts = _packet.first().getTimestampUs(_packet.tsOverflowCount());
    if(ts < DVS_START_CAP) return;
    std_msgs::Header hd;
    hd.seq = evt_seq++;
    hd.stamp = ros::Time(ts / 1e6);

    dvs_msgs::EventArray event_msgs;
    event_msgs.header = hd;
    event_msgs.height = dvs_height;
    event_msgs.width = dvs_width;

    for (auto &evt : _packet) {
        // Get the timestamp of the event.
        iness::time::TimeUs ts = evt.getTimestampUs(_packet.tsOverflowCount());

        dvs_msgs::Event event_msg;
        event_msg.ts = ros::Time(ts / 1e6);
        event_msg.polarity = (uint8_t)(evt.getPolarity());
        event_msg.x = evt.getX();
        event_msg.y = evt.getY();
        
        event_msgs.events.push_back(event_msg);
    }

    bag.write("/dvs/events", hd.stamp, event_msgs);
    // printf("e(%lu) ", _packet.size());

}


int DVSMain(const std::string folder){
    bag.open(folder + "-dvs.bag", rosbag::bagmode::Write);

    // Set up the device and processing callbacks.
    iness::device::Sees sees;
    sees.setImuEnabled(true);
    sees.setApsEnabled(true);
    sees.setDvsEnabled(true);
    sees.setAutoExposureEnabled(true);

    sees.registerPolarityEventPacketCallback(std::bind(polarityEventPacketCallback, std::placeholders::_1));
    sees.registerImu6EventPacketCallback(std::bind(ImuPacketCallback, std::placeholders::_1));
    sees.registerFrameEventPacketCallback(std::bind(FrameCallback, std::placeholders::_1));

    // Start the device driver.
    if (!sees.start()){
        return EXIT_FAILURE;
    }

    // Initialize the event image creator.
    dvs_height = sees.dvsHeight();
    dvs_width = sees.dvsWidth();
    

    printf("DVS is running ...\n");
    const int DUR_MS = 1000;
    while (1)
    {
        std::chrono::milliseconds dur(DUR_MS);
        std::this_thread::sleep_for(dur);
    }
    std::cout << "Shutdown successful.\n";

    return EXIT_SUCCESS;
}