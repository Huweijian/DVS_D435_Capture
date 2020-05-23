
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>


#include <iostream>

using namespace cv;
using namespace std;

int D435Main()
{
    // came init
    rs2::pipeline pipe;
    // pip = &pipe;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH);
    cfg.enable_stream(RS2_STREAM_INFRARED);
    auto profile = pipe.start(cfg);

    rs2::device dev = profile.get_device();
    auto sensors = dev.query_sensors();
    auto stereo = sensors[0];
    // 自动曝光
    stereo.set_option(rs2_option::RS2_OPTION_ENABLE_AUTO_EXPOSURE, 1);
    stereo.set_option(rs2_option::RS2_OPTION_EMITTER_ENABLED, 0);

    printf("D435 is running ...\n");


    while (1)
    {
        rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
        rs2::frame infrared = data.get_infrared_frame();

        // Query frame size (width and height)
        const int w = infrared.as<rs2::video_frame>().get_width();
        const int h = infrared.as<rs2::video_frame>().get_height();

        // Create OpenCV matrix of size (w,h) from the colorized infrared data
	    Mat image(Size(w, h), CV_8UC1, (void*)infrared.get_data(), Mat::AUTO_STEP);
        cv::resize(image, image, Size(), 0.25, 0.25);

        // Update the window with new data
        cv::imshow("D435", image);
        cv::waitKey(1);
    }
    return 0;
}