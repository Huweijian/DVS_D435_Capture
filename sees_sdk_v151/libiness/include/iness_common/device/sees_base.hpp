/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Wed Thu 23 2017
 */

#ifndef INESS_COMMON_DEVICE_SEES_BASE_HPP_
#define INESS_COMMON_DEVICE_SEES_BASE_HPP_


#include "iness_common/types/events/ser_events/event_packet_container_ptr.hpp"
#include "iness_common/device/i_aedat_source.hpp"

namespace iness
{
namespace device
{

/*! Abstract interface definition for Silicon Eye Event Sensors (SEES)
 *
 */
class SeesBase: public aedat::IAedatSource
{
public:
  virtual ~SeesBase()=default;

  /*! Registers a container level callback
   */
  INESS_DEPRECATED virtual void registerContainerCallback( std::function<void(SeesContainerPtr)> _func ){ registerCallback(_func); };

  /*! Registers a container level callback
   */
  virtual void registerCallback( std::function<void(SeesContainerPtr)> _func )=0;

  /*! Register new polarity event callback function. Gets called per polarity event.
   */
  INESS_DEPRECATED virtual void registerPolarityEventCallback( std::function<void(PolarityEvent&, PolarityEventPacket&)> _func ){ registerCallback(_func); };

  /*! Register new polarity event callback function. Gets called per polarity event.
   */
  virtual void registerCallback( std::function<void(PolarityEvent&, PolarityEventPacket&)> _func )=0;

  /*! Register new polarity event packet callback function. Gets called per polarity event packet.
   */
  INESS_DEPRECATED virtual void registerPolarityEventPacketCallback( std::function<void(PolarityEventPacket&)> _func ){ registerCallback(_func); };

  /*! Register new polarity event packet callback function. Gets called per polarity event packet.
   */
  virtual void registerCallback( std::function<void(PolarityEventPacket&)> _func )=0;

  /*! Register new frame event callback function. Gets called per polarity event.
   */
  INESS_DEPRECATED virtual void registerFrameEventCallback( std::function<void(FrameEvent&,FrameEventPacket&)> _func ){ registerCallback(_func); };

  /*! Register new frame event callback function. Gets called per polarity event.
   */
  virtual void registerCallback( std::function<void(FrameEvent&,FrameEventPacket&)> _func )=0;

  /*! Register new frame event packet callback function. Gets called per polarity event packet.
   *
   */
  INESS_DEPRECATED virtual void registerFrameEventPacketCallback( std::function<void(FrameEventPacket&)> _func ){ registerCallback(_func); };

  /*! Register new frame event packet callback function. Gets called per polarity event packet.
   *
   */
  virtual void registerCallback( std::function<void(FrameEventPacket&)> _func )=0;



  /*! Register new IMU6 event callback function. Gets called per polarity event.
   */
  INESS_DEPRECATED virtual void registerImu6EventCallback( std::function<void(Imu6Event&,EventPacket<Imu6Event>&)> _func ){ registerCallback(_func); };



  /*! Register new IMU6 event callback function. Gets called per polarity event.
   */
  virtual void registerCallback( std::function<void(Imu6Event&,EventPacket<Imu6Event>&)> _func )=0;

  /*! Register new IMU6 event packet callback function. Gets called per polarity event packet.
   *
   */
  INESS_DEPRECATED virtual void registerImu6EventPacketCallback( std::function<void(EventPacket<Imu6Event>&)> _func ){ registerCallback(_func); };

  /*! Register new IMU6 event packet callback function. Gets called per polarity event packet.
   *
   */
  virtual void registerCallback( std::function<void(EventPacket<Imu6Event>&)> _func )=0;



  /*! Register new special event callback function. Gets called per polarity event.
   */
  INESS_DEPRECATED virtual void registerSpecialEventCallback( std::function<void(SpecialEvent&,EventPacket<SpecialEvent>&)> _func ){ registerCallback(_func); };

  /*! Register new special event callback function. Gets called per polarity event.
   */
  virtual void registerCallback( std::function<void(SpecialEvent&,EventPacket<SpecialEvent>&)> _func )=0;

  /*! Register new special event packet callback function. Gets called per polarity event packet.
   *
   */
  INESS_DEPRECATED virtual void registerSpecialEventPacketCallback( std::function<void(EventPacket<SpecialEvent>&)> _func ){ registerCallback(_func); };

  /*! Register new special event packet callback function. Gets called per polarity event packet.
   *
   */
  virtual void registerCallback( std::function<void(EventPacket<SpecialEvent>&)> _func )=0;

};

}
}



#endif
