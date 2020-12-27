/*
 * sees_packet.hpp
 *
 *  Created on: Mar 7, 2017
 *      Author: S. Isler (stefan@insightness.com), Insightness AG
 */

#ifndef TYPES_SEES_CONTAINER_HPP_
#define TYPES_SEES_CONTAINER_HPP_

#include "iness_common/types/events/ser_events/event_packet.h"
#include "iness_common/types/events/ser_events/polarity_event.h"
#include "iness_common/types/events/ser_events/frame_event.h"
#include "iness_common/types/events/ser_events/imu6_event.h"
#include "iness_common/types/events/ser_events/special_event.h"

namespace iness
{

class SeesContainer;
typedef std::shared_ptr<SeesContainer> SeesContainerPtr;

/*! Abstract interface definition for sees data containers
 */
class SeesContainer
{
public:
  //! Default constructor
  SeesContainer() = default;

  //! virtual destructor
  virtual ~SeesContainer(){};

  //! Returns true if the given type exists within the container.
  virtual bool hasType( const serialization::AedatType& _type ) const=0;

  //! Returns true if container has polarity events.
  virtual bool hasPolarityEvents() const=0;

  //! Returns true if container has imu events.
  virtual bool hasImu6Events() const=0;

  //! Returns true if container has frames.
  virtual bool hasFrameEvents() const=0;

  //! Returns true if container has special events.
  virtual bool hasSpecialEvents() const=0;

  //! Returns the number of event packets within the container.
  virtual int32_t eventPacketNr() const=0;

  //! Returns the polarity event packet.
  virtual PolarityEventPacket* getPolarityEvents()=0;
  virtual const PolarityEventPacket* getPolarityEvents() const =0;

  /*! Return pointer to the imu packet.
   * cpp-style wrappers similar to the event packets.
   */
  virtual Imu6EventPacket* getImu6Events() =0;
  virtual const Imu6EventPacket* getImu6Events() const =0;

  /*! Return frame packet
   * cpp-style wrappers similar to the event packets.
   */
  virtual FrameEventPacket* getFrameEvents() =0;
  virtual const FrameEventPacket* getFrameEvents() const =0;

  /*! Return pointer to the special events packet.
   * cpp-style wrappers similar to the event packets.
   */
  virtual SpecialEventPacket* getSpecialEvents() =0;
  virtual const SpecialEventPacket* getSpecialEvents() const =0;

  //! Returns true if event_packet pointers are available
  virtual bool hasPackets() const=0;
};

}



#endif /* TYPES_SEES_CONTAINER_HPP_ */
