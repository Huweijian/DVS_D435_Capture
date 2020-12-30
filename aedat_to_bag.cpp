#include <iness_common/device/sees/sees.hpp>
#include <iness_common/streaming/aedat_reader/aedat_input.hpp>
#include <iness_common/streaming/aedat_reader/simple_file_reader.hpp>

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
#include <unordered_map>

using namespace std;
u_int32_t t0 = 0;
std::chrono::high_resolution_clock::time_point tp0;

unsigned int dvs_height;
unsigned int dvs_width;
unordered_map<double, double> ts_to_utc;
string aedat_file ;
string utc_file ;

inline uint32_t utc_us_in_a_day(void){
    using namespace std;
    auto tp1 = chrono::high_resolution_clock().now();
    double dt = chrono::duration_cast<chrono::milliseconds>(tp1-tp0).count();
    uint32_t ts = t0 + (uint32_t) (dt);
    return ts;
}

inline void ImuPacketCallback(rosbag::Bag &bag, iness::Imu6EventPacket &_packet) {
    static unsigned int imu_seq = 0;
    double ts;
    for (auto &event : _packet) {
        // Get the timestamp of the event.
        ts = event.getTimestampUs(_packet.header().event_ts_overflow) / 1e6;

        sensor_msgs::Imu imu;
        imu.header.seq = imu_seq++;
        imu.header.stamp = ros::Time(ts );
        imu.linear_acceleration.x = event.getAccelerationX() * 9.81;
        imu.linear_acceleration.y = event.getAccelerationY() * 9.81;
        imu.linear_acceleration.z = event.getAccelerationZ() * 9.81;
        imu.angular_velocity.x = event.getGyroX() / 180.0 * M_PI;
        imu.angular_velocity.y = event.getGyroY() / 180.0 * M_PI;
        imu.angular_velocity.z = event.getGyroZ() / 180.0 * M_PI;

        bag.write("/dvs/imu", imu.header.stamp, imu);
        printf("IMU: %.3f\n", ts);
    }
}

inline void frameEventPacketCallback(rosbag::Bag &bag,iness::FrameEventPacket &_packet) {
    using namespace cv;
    static unsigned int frame_seq = 0;
   for (auto &frm : _packet) {
        double ts = frm.getTimestampUs(_packet.header().event_ts_overflow) / 1e6;
        printf(" - Frame: %lu at %.2f\n",_packet.size(), ts);

        Mat frm_img = frm.image();
        if (frm_img.empty()) {
            printf(" * WARNING! empty frame\n");
            continue;
        }
        imshow("frame", frm_img); waitKey(5);

        if(ts_to_utc.count(ts) == 0 && utc_file.empty() == false){
            printf("warning: you input a utc file, but its ts doesn't match with the adeat file\n") ;
        }

        double utc = ts_to_utc[ts];
        std_msgs::Header hd;
        hd.seq = frame_seq++;
        hd.stamp = ros::Time(ts);
        hd.seq = (uint32_t)(fmod(utc, 60*60*24) * 1000.0); // utc(s) to ms in a day
        cout << "utc in a day :" << hd.seq << endl;
        cv_bridge::CvImage img_tmp(hd, "mono16", frm_img);
        sensor_msgs::Image img_msg;
        img_tmp.toImageMsg(img_msg);
        bag.write("/dvs/image_raw", img_msg.header.stamp, img_msg);
    }
    return;
}

inline void polarityEventPacketCallback(rosbag::Bag &bag,iness::PolarityEventPacket &_packet) {
    static unsigned int evt_seq = 0;
    double ts = _packet.first().getTimestampUs(_packet.tsOverflowCount()) / 1e6;

    printf("Event: %lu at %.2f\n", _packet.size(), ts);
    int n_event_buf = _packet.size();
    std_msgs::Header hd;
    hd.seq = evt_seq++;
    hd.stamp = ros::Time(ts);
    dvs_msgs::EventArray event_msgs;
    event_msgs.header = hd;
    event_msgs.height = dvs_height;
    event_msgs.width = dvs_width;
    event_msgs.events.reserve(n_event_buf);
    for (auto &evt : _packet) {
        double e_ts = evt.getTimestampUs(_packet.tsOverflowCount()) / 1e6;
        if (e_ts < 0) {
            printf("%.2f\n", e_ts);
            continue;
        }
        dvs_msgs::Event e;
        e.ts = ros::Time(e_ts);
        e.polarity = (uint8_t)(evt.getPolarity());
        e.x = evt.getX();
        e.y = evt.getY();
        event_msgs.events.push_back(e);
    }
    bag.write("/dvs/events", event_msgs.header.stamp, event_msgs);
    return;
}

int main(int argc, char **argv) {
    if(argc  != 2 && argc != 3){
        printf("usage: aedat_to_bag xxx.aedat [utc.txt]\n");
        return 0;
    }

    aedat_file = argv[1];
    if (argc == 3) {
        utc_file = argv[2];
        ifstream utc_in(utc_file);
        cout << utc_file << endl;
        double ts, utc;
        while (utc_in >> ts >> utc) {
            // cout << setprecision(13) << ts << " " << utc << endl;
            ts_to_utc[ts] = utc;
        }
    }

    iness::aedat::SimpleFileReader reader(aedat_file);

    string bag_name = aedat_file + ".bag";
    rosbag::Bag bag;
    bag.open(bag_name, rosbag::bagmode::Write);

    unsigned int width, height;
    auto file_info = reader.getSourceInfo();
    iness::ptree::read(file_info[0].source_info->entry("DvsWidth"), width);
    iness::ptree::read(file_info[0].source_info->entry("DvsHeight"), height);
    cout << width << " x " << height << endl;

    while (true) {
        auto next_packet = reader.next();

        if (!next_packet)  // no more packets
            break;

        if (next_packet->is(iness::serialization::AedatType::POLARITY_EVENT))
            polarityEventPacketCallback(bag, *next_packet->interpretAs<iness::PolarityEventPacket>());
        else if (next_packet->is(iness::serialization::AedatType::FRAME_EVENT))
            frameEventPacketCallback(bag, *next_packet->interpretAs<iness::FrameEventPacket>());
        else if (next_packet->is(iness::serialization::AedatType::IMU6_EVENT)) {
            ImuPacketCallback(bag, *next_packet->interpretAs<iness::Imu6EventPacket>());
        }
    }

    bag.close();
    std::cout << "Shutdown successful.\n";

    return 0;
}