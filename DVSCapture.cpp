#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <signal.h>
#include <ostream>
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <sydevmanager.h>

// Some global variables.
std::atomic<bool> globalShutdown_g; // To trigger the shutdown of the process.
cv::Mat frame_image_g;              // Most recent frame_image_g read from the driver.
std::mutex frame_image_mutex_g;     // Mutex to protect the frame image access.

// SimpleEventImage accumulates events into an image until the image is read.
struct BinPacket
{
    uint64_t ts;
    uint16_t x;
    uint16_t y;
    unsigned char polar;
};

    std::mutex event_image_mutex;
    std::ofstream event_writer;

    // // Reads the image and resets it.
    // cv::Mat getImageAndReset()
    // {
    //     std::lock_guard<std::mutex> lock(event_image_mutex);
    //     cv::Mat output_image = event_image.clone();
    //     event_image = cv::Mat(event_image.rows, event_image.cols, CV_8UC1, cv::Scalar(127));
    //     return output_image;
    // }


// ------------------------------------

void getFrameDVS(Dvs_t * dvs)
{
    // memcpy(dvsdata,dvs->img.data,640*480);
    // dvs->img.setTo(cv::Scalar(128));
    return;
}

void getDVSRaw(uchar *DVSRaw, int length) {
    // std::lock_guard<std::mutex> lock(event_image_mutex);

    printf("%d data get!\n", length);
    for (int i=0; i<length; i++) {

        // Get the pixel coordinate of the event.
        
        uint16_t x = 0;
        uint16_t y = 0;
        uchar time = DVSRaw[i*12];
        printf("%0xd\n", time);

        // // Get the polarity of the event.
        // const iness::Polarity polarity = event.getPolarity();
        // BinPacket bin_data;
        // bin_data.ts = event.getTimestampUs(_packet.tsOverflowCount());
        // bin_data.x = x;
        // bin_data.y = y;
        // bin_data.polar = (unsigned char)polarity;
        // event_writer.write((char *)(&bin_data), 13);

        // // Display positive polarity events white and negative polarity events black.
        // if (polarity == iness::Polarity::POS)
        //     event_image.at<uchar>(y, x) = 255;
        // else
        //     event_image.at<uchar>(y, x) = 0;

    }
    return;
}

int DVSMain(const std::string folder){
    using namespace std;
    Sydevmanager *deviceManager = nullptr;
    if (!deviceManager->OpenDVSCamera((char *)"/dev/video5", getFrameDVS, getDVSRaw)){
        printf("DVS init failed!\n");
    }
	string folder_img = (folder + "/DVS_Img");
    mkdir(folder_img.c_str(), ACCESSPERMS);
    printf("DVS is running ...\n");

    cv::Mat event_img;
    int cnt = 0;
    const int DUR_MS = 100;

    // event_writer(folder+"/DVS.bin", std::ios::binary)

    while (1)
    {

        // event_img = simple_event_image_ptr->getImageAndReset();
        // if (!event_img.empty()){
        //     char img_idx[10] = "";
        //     sprintf(img_idx, "%05d", cnt);
        //     imwrite(folder_img + "/" + string(img_idx) + ".png", event_img);
        // }
        // int pack_n = simple_event_image_ptr->getPacketNum();
        std::chrono::milliseconds dur(DUR_MS);
        std::this_thread::sleep_for(dur);
        // printf("DVS\t%d (%d packs in %d ms)\n", cnt++, pack_n, DUR_MS);

        
    }
    std::cout << "Shutdown successful.\n";

    return EXIT_SUCCESS;

}