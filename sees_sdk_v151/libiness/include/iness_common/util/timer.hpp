/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Jonas Strubel <jonas@insightness.com>, May 2017
*/

#ifndef TIMER_H
#define TIMER_H

#include <thread>
#include <boost/asio.hpp>
#include "iness_common/types/eventhandler.h"

namespace iness {

class Timer
{
public:
    Timer(bool _periodic = false);
    ~Timer();

    void start(uint64_t _milliseconds);
    void stop();
    void setPeriodic(bool _periodic = true);

    EventHandler<> timeout;

private:
    void run();
    void tick();
    void handleTimeout(const boost::system::error_code&_error);

    bool periodic_;
    std::thread thread_;
    uint64_t milliseconds_;

    boost::asio::io_service io_service_;
    boost::asio::deadline_timer timer_;
    std::unique_ptr<boost::asio::io_service::work> work_;

    bool enabled_;
};

}

#endif // TIMER_H
