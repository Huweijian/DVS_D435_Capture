/*
 * event_packet_container_ptr.hpp
 *
 *  Created on: Mar 14, 2017
 *      Author: S. Isler (stefan@insightness.com), Insightness AG
 */

#ifndef TYPES_SER_EVENTS_EVENT_PACKET_CONTAINER_PTR_HPP_
#define TYPES_SER_EVENTS_EVENT_PACKET_CONTAINER_PTR_HPP_

#include <memory>

#include "iness_common/types/events/ser_events/event_packet_container.h"
#include "iness_common/types/sees_container.hpp"

namespace iness
{

class EventPacketContainerHandler;

typedef std::shared_ptr<EventPacketContainerHandler> SharedEventPacketContainerPtr;

/*! Handler class for (libcaer) EventPacketContainer pointers that can then be used with c++ smart pointers.
 *
 */
class EventPacketContainerHandler: public SeesContainer
{
public:
  //! Constructor for raw libcaer pointers.
  EventPacketContainerHandler( caer_event_packet_container* _ptr );

  //! Constructor for our c++ libcaer wrapper container class.
  EventPacketContainerHandler( EventPacketContainer* _ptr = nullptr );

  //! Destructor
  ~EventPacketContainerHandler();

  //! Returns true if the internal pointer is null
  bool isNull() const;


  //! Returns true if the given type exists within the container.
  virtual bool hasType( const serialization::AedatType& _type ) const override;

  //! Returns true if container has polarity events.
  virtual bool hasPolarityEvents() const override;

  //! Returns true if container has imu events.
  virtual bool hasImu6Events() const override;

  //! Returns true if container has frames.
  virtual bool hasFrameEvents() const override;

  //! Returns true if container has special events.
  virtual bool hasSpecialEvents() const override;

  //! Returns the number of event packets within the container.
  virtual int32_t eventPacketNr() const override;

  //! Returns the polarity event packet.
  virtual PolarityEventPacket* getPolarityEvents() override;
  virtual const PolarityEventPacket* getPolarityEvents() const override;

  /*! Return pointer to the imu packet.
   * cpp-style wrappers similar to the event packets.
   */
  virtual Imu6EventPacket* getImu6Events() override;
  virtual const Imu6EventPacket* getImu6Events() const override;

  /*! Return frame packet
   * cpp-style wrappers similar to the event packets.
   */
  virtual FrameEventPacket* getFrameEvents() override;
  virtual const FrameEventPacket* getFrameEvents() const override;

  /*! Return pointer to the special events packet.
   * cpp-style wrappers similar to the event packets.
   */
  virtual SpecialEventPacket* getSpecialEvents() override;
  virtual const SpecialEventPacket* getSpecialEvents() const override;

  //! Returns true if event_packet pointers are available
  virtual bool hasPackets() const override;

  /*! Returns a pointer to the container itself.
   */
  EventPacketContainer* container();

private:
  EventPacketContainer* data_; //! Pointer to the actual data.
};

}


#endif /* TYPES_SER_EVENTS_EVENT_PACKET_CONTAINER_PTR_HPP_ */
