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
#include <deque>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <rosbag/bag.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/Image.h>
#include <dvs_msgs/EventArray.h>
#include <cv_bridge/cv_bridge.h>
#include <atomic>

std::atomic_bool is_shutdown;
std::vector<sensor_msgs::Image> msg_img_buf;
std::vector<sensor_msgs::Imu> msg_imu_buf;
dvs_msgs::EventArray msg_event_buf[1000];
int n_event_buf = 0;
std::mutex m_img, m_imu, m_evt;
u_int32_t t0 = 0;
std::chrono::high_resolution_clock::time_point tp0;

int dvs_height;
int dvs_width;
const unsigned int DVS_START_CAP = 5e6; // 1 sec

inline uint32_t utc_us_in_a_day(void){
    using namespace std;
    auto tp1 = chrono::high_resolution_clock().now();
    double dt = chrono::duration_cast<chrono::milliseconds>(tp1-tp0).count();
    uint32_t ts = t0 + (uint32_t) (dt);
    return ts;
}

void ImuPacketCallback(iness::Imu6EventPacket &_packet)
{
    static int last_sec = -1;
    static unsigned int imu_seq = 0;

    std::lock_guard<std::mutex> lck(m_imu);
    for(auto& event : _packet)
    {
        // Get the timestamp of the event.
        iness::time::TimeUs ts = event.getTimestampUs(_packet.header().event_ts_overflow);
        if(ts < DVS_START_CAP) continue;
        sensor_msgs::Imu imu;
        imu.header.seq = imu_seq++;
        imu.header.stamp = ros::Time(ts/1e6);
        imu.linear_acceleration.x = event.getAccelerationX() * 9.81;
        imu.linear_acceleration.y = event.getAccelerationY() * 9.81;
        imu.linear_acceleration.z = event.getAccelerationZ() * 9.81;
        imu.angular_velocity.x = event.getGyroX() / 180.0 * M_PI;
        imu.angular_velocity.y = event.getGyroY() / 180.0 * M_PI;
        imu.angular_velocity.z = event.getGyroZ() / 180.0 * M_PI;;
        msg_imu_buf.emplace_back(imu);

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

    std::lock_guard<std::mutex> lck(m_img);
    for(auto& frm : _packet)
    {
        iness::time::TimeUs ts = frm.getTimestampUs(_packet.header().event_ts_overflow);
        uint32_t utc = utc_us_in_a_day();

        Mat img = frm.getImage();
        if(img.empty()) {
            printf(" * WARNING! empty frame\n");
            continue;
        }

        if(ts < DVS_START_CAP) {
            putText(img, std::to_string(ts/1e6), {100, 100}, FONT_HERSHEY_PLAIN, 3.0, 65535);
        }

        std_msgs::Header hd;
        hd.seq = frame_seq++;
        hd.stamp = ros::Time(ts / 1e6);
        hd.seq = utc; //TODO 这样对齐不太好
        
        cv_bridge::CvImage img_tmp(hd, "mono16", img);
        sensor_msgs::Image img_msg;
        img_tmp.toImageMsg(img_msg);
        msg_img_buf.emplace_back(img_msg);
    }
}

void polarityEventPacketCallback(iness::PolarityEventPacket &_packet)
{
    static unsigned int evt_seq = 0;
    std::lock_guard<std::mutex> lck(m_evt);
    iness::time::TimeUs ts = _packet.first().getTimestampUs(_packet.tsOverflowCount());
    if(ts < DVS_START_CAP) return;

    std_msgs::Header hd;
    hd.seq = evt_seq++;
    hd.stamp = ros::Time(ts / 1e6);
    dvs_msgs::EventArray &event_msgs = msg_event_buf[n_event_buf];
    event_msgs.header = hd;
    event_msgs.height = dvs_height;
    event_msgs.width = dvs_width;

    dvs_msgs::Event evts[5000];
    int i = 0;

    for (auto &evt : _packet) {
        // Get the timestamp of the event.
        iness::time::TimeUs ts = evt.getTimestampUs(_packet.tsOverflowCount());
        evts[i].ts = ros::Time(ts / 1e6);
        evts[i].polarity = (uint8_t)(evt.getPolarity());
        evts[i].x = evt.getX();
        evts[i].y = evt.getY();
        i++; 
    }
    int packet_size = _packet.size();
    std::vector<dvs_msgs::Event> tmp(begin(evts), begin(evts)+packet_size);
    event_msgs.events = std::move(tmp);
    n_event_buf++;
    // printf("e(%lu) ", _packet.size());

}



int DVSMain(const std::string folder){

    std::chrono::milliseconds slp(100);
    for(int i=0; i<10; i++){
        std::this_thread::sleep_for(slp);
        auto tp0_tmp = std::chrono::high_resolution_clock().now();
        tp0 = std::chrono::high_resolution_clock::from_time_t(std::chrono::high_resolution_clock::to_time_t(tp0_tmp));
        t0 = (std::chrono::high_resolution_clock::to_time_t(tp0) % (60*60*24)) * 1e3;
    }
    printf("UTC: %d sec (%d:%d:%d)\n", t0/1000, 8+(t0/1000/3600), (t0/1000%3600/60), (t0/1000%3600%60));

    rosbag::Bag bag;
    bag.open(folder + "-dvs.bag", rosbag::bagmode::Write);

    // Set up the device and processing callbacks.
    iness::device::Sees sees;
    sees.resetDeviceTime();
    sees.setImuEnabled(true);
    sees.setApsEnabled(true);
    sees.setDvsEnabled(true);
    sees.setAutoExposureEnabled(true);
    // sees.setAutoExposureMedianBrightness(0.6);
    sees.setEventThreshold(55);

    sees.registerCallback(std::bind(polarityEventPacketCallback, std::placeholders::_1));
    sees.registerCallback(std::bind(ImuPacketCallback, std::placeholders::_1));
    sees.registerCallback(std::bind(FrameCallback, std::placeholders::_1));

    // Start the device driver.
    if (!sees.start()){
        return EXIT_FAILURE;
    }

    // Initialize the event image creator.
    dvs_height = sees.dvsHeight();
    dvs_width = sees.dvsWidth();
    cv::namedWindow("img");
    
    printf("DVS is running ...\n");
    is_shutdown = false;
    while (!is_shutdown)
    {
        {
            std::lock_guard<std::mutex> lck(m_evt);
            // printf("evtBuf: size = %d\n", n_event_buf);
            for(int i=0; i<n_event_buf; i++){
                auto &event_msgs = msg_event_buf[i];
                bag.write("/dvs/events", event_msgs.header.stamp, event_msgs);
            }
            n_event_buf = 0;
        }
        {
            std::lock_guard<std::mutex> lck(m_imu);
            for(auto & imu : msg_imu_buf) 
                bag.write("/dvs/imu",imu.header.stamp, imu);
            msg_imu_buf.clear();

        }
        {
            std::lock_guard<std::mutex> lck(m_img);
            for(auto &img_msg : msg_img_buf){
                if(img_msg.header.stamp.toSec() < DVS_START_CAP/1e6){
                    printf("skippppppppp a image at %f\n", img_msg.header.stamp.toSec() );
                    continue;
                }
                bag.write("/dvs/image_raw", img_msg.header.stamp, img_msg);
            }
            
            if(!msg_img_buf.empty()){
                cv_bridge::CvImageConstPtr cv_ptr;
                cv_ptr = cv_bridge::toCvCopy((msg_img_buf.back()), "mono16");
                cv::imshow("img", cv_ptr->image);
                if(cv::waitKey(1) == 'q'){
                    is_shutdown = true;
                }
                msg_img_buf.clear();
            }        
        }
        std::chrono::milliseconds dur(30);
        std::this_thread::sleep_for(dur);
    }
    bag.close();
    sees.stop();
    std::cout << "Shutdown successful.\n";
    cv::destroyAllWindows();

    return EXIT_SUCCESS;
}