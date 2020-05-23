#include <iness_common/device/sees/sees.hpp>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <signal.h>
#include <ostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <librealsense2/rs.hpp>

// Some global variables.
std::atomic<bool> globalShutdown; // To trigger the shutdown of the process.
cv::Mat frame_image;              // Most recent frame_image read from the driver.
std::mutex frame_image_mutex;     // Mutex to protect the frame image access.

// SimpleEventImage accumulates events into an image until the image is read.
struct BinPacket
{
    uint64_t ts;
    uint16_t x;
    uint16_t y;
    unsigned char polar;
};

class SimpleEventImage
{
  public:
    SimpleEventImage() = delete;
    SimpleEventImage(const uint16_t _sensor_height, const uint16_t _sensor_width) : event_image(_sensor_height, _sensor_width, CV_8UC1, cv::Scalar(127)), event_writer("out.data", std::ios::binary){};

    // Adds events to the image.
    void processEventPacket(const iness::PolarityEventPacket &_packet)
    {
        std::lock_guard<std::mutex> lock(event_image_mutex);
        for (auto &event : _packet)
        {
            // Get the pixel coordinate of the event.
            const uint16_t x = event.getX();
            const uint16_t y = event.getY();
            // Get the polarity of the event.
            const iness::Polarity polarity = event.getPolarity();
            BinPacket bin_data;
            bin_data.ts = event.getTimestampUs(_packet.tsOverflowCount());
            bin_data.x = x;
            bin_data.y = y;
            bin_data.polar = (unsigned char)polarity;
            event_writer.write((char *)(&bin_data), 13);
            // Display positive polarity events white and negative polarity events black.
            if (polarity == iness::Polarity::POS)
            {
                event_image.at<uchar>(y, x) = 255;
            }
            else
            {
                event_image.at<uchar>(y, x) = 0;
            }
        }
    }

    // Reads the image and resets it.
    cv::Mat getImageAndReset()
    {
        std::lock_guard<std::mutex> lock(event_image_mutex);
        cv::Mat output_image = event_image.clone();
        event_image = cv::Mat(event_image.rows, event_image.cols, CV_8UC1, cv::Scalar(127));
        return output_image;
    }

  private:
    cv::Mat event_image;
    std::mutex event_image_mutex;
    std::ofstream event_writer;
};

std::shared_ptr<SimpleEventImage> simple_event_image_ptr;

// Thread safe frame_image image setter.
void setFrameImage(const cv::Mat &_new_frame_image)
{
    std::lock_guard<std::mutex> lock(frame_image_mutex);
    frame_image = _new_frame_image;
}

// Thread safe frame_image image getter.
void getFrameImage(cv::Mat &_frame_image)
{
    std::lock_guard<std::mutex> lock(frame_image_mutex);
    _frame_image = frame_image;
}

void polarityEventPacketCallback(iness::PolarityEventPacket &_packet)
{
    // How to iterate through the event packet and retrieve the timestamp of the events.
    // for(auto& event : _packet)
    // {
    //  // Get the timestamp of the event.
    //  iness::time::TimeUs ts = event.getTimestampUs(_packet.tsOverflowCount());
    //  // Get the pixel coordinates of the event.
    //  uint16_t x = event.getX();
    //  uint16_t y = event.getY();
    //}

    // Process packet for the event image visualization.
    if (simple_event_image_ptr)
    {
        simple_event_image_ptr->processEventPacket(_packet);
    }
}

static void globalShutdownSignalHandler(int _signal)
{
    // Simply set the running flag to false on SIGTERM and SIGINT (CTRL+C) for global shutdown.
    if (_signal == SIGTERM || _signal == SIGINT)
    {
        globalShutdown = true;
    }
}

bool setUpShutdownSignalHandler()
{
    // Install signal handler for global shutdown.
    globalShutdown = false;
    struct sigaction shutdownAction;
    shutdownAction.sa_handler = &globalShutdownSignalHandler;
    shutdownAction.sa_flags = 0;
    sigemptyset(&shutdownAction.sa_mask);
    sigaddset(&shutdownAction.sa_mask, SIGTERM);
    sigaddset(&shutdownAction.sa_mask, SIGINT);

    if (sigaction(SIGTERM, &shutdownAction, NULL) == -1)
    {
        return false;
    }

    if (sigaction(SIGINT, &shutdownAction, NULL) == -1)
    {
        return false;
    }

    return true;
}

int main(void)
{
    using namespace cv;

    if (0)
    {

        // Declare RealSense pipeline, encapsulating the actual device and sensors
        rs2::pipeline pipe;
        // Start streaming with default recommended configuration
        pipe.start();
        while (1)
        {
            rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
            rs2::frame infrared = data.get_infrared_frame(0);

            // Query frame size (width and height)
            const int w = infrared.as<rs2::video_frame>().get_width();
            const int h = infrared.as<rs2::video_frame>().get_height();

            // Create OpenCV matrix of size (w,h) from the colorized infrared data
            Mat image(Size(w, h), CV_16UC1, (void *)infrared.get_data(), Mat::AUTO_STEP);

            // Update the window with new data
            imshow("hi", image);
            waitKey(1);
        }
        return 0;
    }

    // Install signal handler for global shutdown.
    if (!setUpShutdownSignalHandler())
    {
        return EXIT_FAILURE;
    }
    // Set up the device and processing callbacks.
    iness::device::Sees sees;
    sees.setImuEnabled(false);
    sees.setApsEnabled(false);
    sees.registerPolarityEventPacketCallback(std::bind(polarityEventPacketCallback, std::placeholders::_1));
    // Start the device driver.
    if (!sees.start())
    {
        return EXIT_FAILURE;
    }
    // Initialize the event image creator.
    simple_event_image_ptr = std::make_shared<SimpleEventImage>(sees.dvsHeight(), sees.dvsWidth());

    cv::Mat event_img;
    while (!globalShutdown)
    {
        event_img = simple_event_image_ptr->getImageAndReset();
        if (!event_img.empty())
        {
            cv::imshow("Event", event_img);
        }
        cv::waitKey(30);
    }
    std::cout << "Shutdown successful.\n";

    return EXIT_SUCCESS;
}
