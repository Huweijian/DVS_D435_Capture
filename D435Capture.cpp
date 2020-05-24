
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <experimental/filesystem>
#include <sys/stat.h>


using namespace cv;
using namespace std;

int D435Main(const string folder)
{
    // came init
    rs2::pipeline pipe;
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

    // writer
	string folder_img = (folder + "/D435_Img");
    mkdir(folder_img.c_str(), ACCESSPERMS);
    
    ofstream of(string(folder)+"/D435_time.txt");

    // S.T.A.R.T
    printf("D435 is running ...\n");
    int cnt = 0;
    while (1)
    {
        // get image
        rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
        rs2::frame infrared = data.get_infrared_frame();
        const int w = infrared.as<rs2::video_frame>().get_width();
        const int h = infrared.as<rs2::video_frame>().get_height();
        Mat image(Size(w, h), CV_8UC1, (void *)infrared.get_data(), Mat::AUTO_STEP);

        // get expo and stamp
        double expo = 0;
        long long stamp = 0;
        try{
            expo = infrared.get_frame_metadata(rs2_frame_metadata_value::RS2_FRAME_METADATA_ACTUAL_EXPOSURE) / 1000.0;
        }
        catch (std::exception &e){
            cout << e.what() << endl;
        }
        stamp = infrared.get_frame_metadata(rs2_frame_metadata_value::RS2_FRAME_METADATA_FRAME_TIMESTAMP);

        // write
        char msg[100] = "";
        sprintf(msg, "%05d %lld %.5f", cnt, stamp, expo);
        of << msg << endl;

        char img_idx[10] = "";
        sprintf(img_idx, "%05d", cnt);
        imwrite(folder_img + "/" + string(img_idx) + ".png", image);
        cnt++;

        // show
        Mat image_show;
        cv::resize(image, image_show, Size(), 0.25, 0.25);
        cv::imshow("D435", image_show);
        int key = waitKey(1);
        if (key == 'q')
            return 0;

    }
    return 0;
}