#ifndef EVENT_PACKET_CONTAINER_H
#define EVENT_PACKET_CONTAINER_H

#include <libcaer-iness/events/packet_container_type.h>
#include "iness_common/types/events/ser_events/event_packet.h"
#include "iness_common/types/events/ser_events/polarity_event.h"
#include "iness_common/types/events/ser_events/frame_event.h"
#include "iness_common/types/events/ser_events/imu6_event.h"
#include "iness_common/types/events/ser_events/special_event.h"


namespace iness {

class LibcaerEventPacket;


typedef caer_event_packet_header EventPacketHeaderRaw;

/*! Wrapper class for libcaer event packet containers
 *
 */
class EventPacketContainer
{
public:
  //! Default constructor
  EventPacketContainer() = default;

  //! Returns true if the given type exists within the container.
  bool hasType( const serialization::AedatType& _type ) const;

  //! Returns true if container has polarity events.
  bool hasPolarityEvents() const;

  //! Returns true if container has imu events.
  bool hasImu6Events() const;

  //! Returns true if container has frames.
  bool hasFrameEvents() const;

  //! Returns true if container has special events.
  bool hasSpecialEvents() const;

  //! Returns the number of event packets within the container.
  int32_t eventPacketNr() const;

  //! Returns the polarity event packet.
  PolarityEventPacket* getPolarityEvents();
  const PolarityEventPacket* getPolarityEvents() const;

  /*! Return pointer to the imu packet.
   * cpp-style wrappers similar to the event packets.
   */
  Imu6EventPacket* getImu6Events();
  const Imu6EventPacket* getImu6Events() const;

  /*! Return frame packet
   * cpp-style wrappers similar to the event packets.
   */
  FrameEventPacket* getFrameEvents();
  const FrameEventPacket* getFrameEvents() const;

  /*! Return pointer to the special events packet.
   * cpp-style wrappers similar to the event packets.
   */
  SpecialEventPacket* getSpecialEvents();
  const SpecialEventPacket* getSpecialEvents() const;

  //! Returns true if event_packet pointers are available
  bool hasPackets() const;

private:
  friend class LibcaerEventPacket;

private:
  int32_t event_packets_nr_{0}; //! Number of different event packets contained - Must be stored in continuous order (If type 3 is present, types 2,1 and 0 must be too)
  EventPacketHeaderRaw* event_packets_{nullptr}; //! Array of pointers to the event packets.
}__attribute__((packed));

}

#endif // EVENT_PACKET_CONTAINER_H
