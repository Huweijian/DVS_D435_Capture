#ifndef EVENT_PACKET_H
#define EVENT_PACKET_H

#include "iness_common/types/events/event_packet_header.h"
#include "iness_common/types/events/ser_events/polarity_event.h"
#include "iness_common/types/events/ser_events/frame_event.h"
#include "iness_common/types/events/ser_events/imu6_event.h"
#include "iness_common/types/events/ser_events/special_event.h"
#include "iness_common/types/events/event_definitions.hpp"
#include "iness_common/types/events/ser_events/templated_event_packet.hpp"

namespace iness {


typedef EventPacket<PolarityEvent> PolarityEventPacket;
typedef EventPacket<Imu6Event> Imu6EventPacket;
typedef EventPacket<SpecialEvent> SpecialEventPacket;
class FrameEventPacket;



class FrameEventPacket
{

public:
    //! Bidirectional iterator to traverse the frame events in the packet (forward decl)
    template<class CRTP_CHILD>
    class IteratorBase;
    class Iterator;
    class ConstIterator;

public:
    FrameEventPacket()=default;

    FrameEvent& at(unsigned int _pos);

    const FrameEvent& at(unsigned int _pos) const;

    //! Direct access to the _pos event
    FrameEvent& operator[]( unsigned int _pos );

    //! Direct access to the _pos event
    const FrameEvent& operator[]( unsigned int _pos ) const;

    //! Returns a reference to the first event in the packet
    FrameEvent& first();

    //! Returns a reference to the first event in the packet
    const FrameEvent& first() const;

    //! Returns a reference to the last event in the packet
    FrameEvent& last();

    //! Returns a reference to the last event in the packet
    const FrameEvent& last() const;

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
    FrameEventPacket(const FrameEventPacket&)=default;

private:
    EventPacketHeader header_;
    //TEvent events_; greyed out because it's not used atm, but frames reside here as well
};



}


#include "impl/frame_event_packet_iterator.inl"

#endif // EVENT_PACKET_H
