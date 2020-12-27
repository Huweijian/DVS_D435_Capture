/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Stefan Isler <stefan@insightness.com>, May 2017
*/

#ifndef DEVICE_I_AEDAT_SOURCE_HPP_
#define DEVICE_I_AEDAT_SOURCE_HPP_

#define INESS_DEPRECATED

#include "iness_common/types/events/i_event_packet.hpp"
#include <functional>

namespace iness
{
namespace aedat
{

/*! Interface class for sources that provide aedat data.
 */
class IAedatSource
{
public:
  IAedatSource() =default;
  virtual ~IAedatSource() =default;
  
  /*! Register a callback that will be called on every packet.
   *  Data in these packets must be protected, i.e. remain
   *  valid throughout the lifetime of the passed IEventPacket type
   * 
   *  @param _cb_func Function to which packets will be passed.
   */
   INESS_DEPRECATED virtual void registerEventPacketCallback( std::function<void(IEventPacketPtr)> _cb_func ){ registerCallback(_cb_func); };
  
  /*! Register a callback that will be called on every packet.
   *  Data in these packets must be protected, i.e. remain
   *  valid throughout the lifetime of the passed IEventPacket type
   * 
   *  @param _cb_func Function to which packets will be passed.
   */
   virtual void registerCallback( std::function<void(IEventPacketPtr)> _cb_func ) =0;
};

}
}



#endif /* DEVICE_I_AEDAT_SOURCE_HPP_ */
