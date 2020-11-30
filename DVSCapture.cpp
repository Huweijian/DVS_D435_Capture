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

// Some global variables.
std::atomic<bool> globalShutdown_g; // To trigger the shutdown of the process.
std::ofstream of;

void ImuPacketCallback(iness::Imu6EventPacket &_packet)
{
    static int last_sec = -1;
    // How to iterate through the event packet and retrieve the timestamp of the events.
    for(auto& event : _packet)
    {
        // Get the timestamp of the event.
        iness::time::TimeUs ts = event.getTimestampUs(_packet.header().event_ts_overflow);
        // Get the pixel coordinates of the event.
        float ax = event.getAccelerationX();
        float ay = event.getAccelerationY();
        float az = event.getAccelerationZ();
        float gx = event.getGyroX();
        float gy = event.getGyroY();
        float gz = event.getGyroZ();
        char tmp[100] = "";
        sprintf(tmp, "%ld, %f, %f, %f, %f, %f, %f\n", ts, ax, ay, az, gx, gy, gz);
        of << tmp;
        if(ts / 1000000 != last_sec){
            last_sec = ts / 1000000;
            printf("Captured %ds: %s", last_sec, tmp);
            if(last_sec > 60*125){
                exit(0);
            }
        }
    }
}

static void globalShutdownSignalHandler(int _signal)
{
    // Simply set the running flag to false on SIGTERM and SIGINT (CTRL+C) for global shutdown.
    if (_signal == SIGTERM || _signal == SIGINT)
    {
        globalShutdown_g = true;
    }
}

bool setUpShutdownSignalHandler()
{
    // Install signal handler for global shutdown.
    globalShutdown_g = false;
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


int DVSMain(const std::string folder){
    using namespace std;
    // Install signal handler for global shutdown.
    if (!setUpShutdownSignalHandler())
    {
        return EXIT_FAILURE;
    }

    of.open(folder+"/imu.txt");

    // Set up the device and processing callbacks.
    iness::device::Sees sees;
    sees.setImuEnabled(true);
    sees.setDvsEnabled(true);
    sees.setApsEnabled(false);
    sees.registerImu6EventPacketCallback(std::bind(ImuPacketCallback, std::placeholders::_1));
    
    // Start the device driver.
    if (!sees.start())
    {
        return EXIT_FAILURE;
    }


    int cnt = 0;
    const int DUR_MS = 1000;

    while (!globalShutdown_g)
    {
        std::chrono::milliseconds dur(DUR_MS);
        std::this_thread::sleep_for(dur);
    }
    std::cout << "Shutdown successful.\n";

    return EXIT_SUCCESS;
}