#ifndef EVENTPACKETHEADER_H
#define EVENTPACKETHEADER_H

#include <stdint.h>
#include "iness_common/types/generated/aedat_type_generated.h"

namespace iness {

#pragma pack(push, 1)
struct EventPacketHeader
{
  serialization::AedatType event_type; //! Type of the events
  int16_t event_source; //! Source of the events, unique inside process.
  int32_t event_size; //! Size of one event [bytes].
  int32_t event_ts_offset; //! Offset from event start to 32-bit timestamp [bytes].
  int32_t event_ts_overflow; //! Timestamp overflow counter (since it's 32 bit internally it overflow about every 36min.
  int32_t event_capacity; //! Max nr of events this packet type can store.
  int32_t event_nr; //! Nr of events within the packet.
  int32_t event_validity; //! Nr of valid events within the packet.
};
#pragma pack(pop)//__attribute__((__packed__));

}

#endif // EVENTPACKETHEADER_H
