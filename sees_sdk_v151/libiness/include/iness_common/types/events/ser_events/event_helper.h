#ifndef EVENTHELPER_H
#define EVENTHELPER_H

#include <stdint.h>
#include <iness_common/time/definitions.hpp>

namespace iness {

constexpr int ts_overflow_shift = 31;

class EventHelper
{
public:
    static time::TimeUs getTimestamp64(int32_t _event_timestamp, int32_t _overflow_timestamp)
    {
        return ((static_cast<time::TimeUs>(_overflow_timestamp))<<ts_overflow_shift) | static_cast<time::TimeUs>(_event_timestamp);
    }
};

}

#endif // EVENTHELPER_H

