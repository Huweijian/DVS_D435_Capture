/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Stefan Isler <stefan@insightness.com>, May 2017
*/

#ifndef TYPES_EVENTS_I_EVENT_PACKET_HPP_
#define TYPES_EVENTS_I_EVENT_PACKET_HPP_

#include "iness_common/types/events/event_packet_header.h"
#include "iness_common/time/definitions.hpp"
#include "iness_common/types/events/ser_events/event_helper.h"
#include <iostream>
#include <memory>
namespace iness
{
namespace aedat
{

/*! Interface class for any event packet type. Interpretation of the data is left to the user.
 *  Any class implementing this interface must guarantee that its data is available and valid for the lifetime
 *  of the object.
 */
class IEventPacket
{
public:
 typedef EventPacketHeader Header;
 typedef std::shared_ptr<IEventPacket> Ptr;
 
public:
  IEventPacket() =default;
  virtual ~IEventPacket() =default;
  
  /*! Returns the packet's header.
   */
  virtual Header* header()
  {
    return reinterpret_cast<Header*>( data() );
  }
  
  /*! Returns a pointer to the data (which includes the header).
   *  The memory pointed to is guaranteed to be valid for the lifetime of the object.
   */
  virtual char* data() =0;
  
  /*! Returns the size of the contained data chunk (including the header)
   */
  virtual unsigned int dataSize()
  {
    return sizeof(Header) + header()->event_size * header()->event_nr;
  }
  
  /*! Interpretes the contained data as a specific event packet type
   */
  template<typename T_EVENT_PACKET_TYPE>
  T_EVENT_PACKET_TYPE* interpretAs()
  {
    return reinterpret_cast<T_EVENT_PACKET_TYPE*>(data());
  }
  
  /*! Convenience function to get the timestamp of the first event.
   */
  virtual iness::time::TimeUs getFirstEventTimestampUs()
  {
    return EventHelper::getTimestamp64(*reinterpret_cast<int32_t*>( data() + sizeof(Header) + header()->event_ts_offset ), header()->event_ts_overflow);
  }

  /*! Returns true if the packet is of the given type
   */
  virtual bool is( iness::serialization::AedatType _type )
  {
    return _type == header()->event_type;
  }
  
};
typedef std::shared_ptr<IEventPacket> IEventPacketPtr;

}
}



#endif /* TYPES_EVENTS_I_EVENT_PACKET_HPP_ */
