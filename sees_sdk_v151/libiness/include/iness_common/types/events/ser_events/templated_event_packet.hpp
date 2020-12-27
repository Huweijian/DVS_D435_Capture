#ifndef TEMPLATED_EVENT_PACKET_H
#define TEMPLATED_EVENT_PACKET_H

#include <vector>

#include "iness_common/types/events/event_definitions.hpp"
#include "iness_common/types/events/event_packet_header.h"
#include "iness_common/types/events/i_event_packet.hpp"

namespace iness {

#pragma pack(push, 1)    
template<typename TEvent>
class EventPacket
{
public:
  typedef TEvent* Iterator;
  typedef const TEvent* ConstIterator;

public:
  //! Default constructor
  EventPacket() = default;

  /**
   * @brief Factory function to create an event packet. The events should all have
   * the same overflow timestamp. Use iness::aedat::getEventTs(iness::time::TimeUs)
   * and iness::aedat::getOverflowTs(iness::time::TimeUs) to separte a 64bit stamp
   * into the upper and lower part.
   * @param _events Vector of event. Will be copied.
   * @param _ts_overflow Timestamp overflow counter (since it's 32 bit in the event it overflow about every 36min.
   * @param _aedat_event_source_id Source of the event, unique inside process.
   * @return Shared pointer to packet.
   */
  static iness::aedat::IEventPacketPtr create(
      const std::vector<TEvent>& _events,
      const uint32_t _ts_overflow,
      const int16_t _aedat_event_source_id);

  //! Direct access to the _pos event
  TEvent& operator[]( unsigned int _pos );

  //! Returns a reference to the first event in the packet
  TEvent& first();

  //! Returns a reference to the last event in the packet
  TEvent& last();

  //! Returns the event packet header.
  EventPacketHeader& header();

  //! Returns the event packet header.
  const EventPacketHeader& header() const;

  //! Returns the number of events (valid and invalid)
  size_t size() const;

  //! Returns the type of the event packet
  serialization::AedatType type() const;

  //! Returns the number of overflows that have occured before, to be used for timestamp correction and conversion from 32bit to 64bit
  int32_t tsOverflowCount() const;

  //! Iterator to first event
  Iterator begin();

  //! End iterator
  Iterator end();

  //! Const iterator to first event
  ConstIterator begin() const;

  //! Const end iterator
  ConstIterator end() const;

private:
  //! Copy constructor is private as long as it is not implemented (it would have to copy the underlying, wrapped data)
  EventPacket(const EventPacket&)=default;

private:
  EventPacketHeader header_; //! Holds information about the package
  TEvent events_; //! Holds the event data.
};
#pragma pack(pop)
}

#endif // TEMPLATED_EVENT_PACKET_H
