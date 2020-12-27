/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Wed Dec 13 2017
 */

#ifndef INESS_COMMON_DEVICE_SEES_SEES_CONFIG_HPP_
#define INESS_COMMON_DEVICE_SEES_SEES_CONFIG_HPP_


#include <cstdint>

namespace iness
{
namespace device
{

//! Provides high level settings for sees devices
struct SeesConfig
{
  bool enable_aps{true}; //!< Whether aps readout is enabled or not
  bool enable_dvs{true}; //!< Whether polarity event readout is enabled or not
  bool enable_imu{true}; //!< Whether imu event readout is enabled or not

  uint32_t exposure_time_us{5000}; //!< Exposure time for aps frames.
  uint32_t frame_rate_hz{30}; //!< Frame rate in hz
  uint32_t event_threshold{42}; //!< Event threshold - Controls the sensitivity of the device to changes. Currently this is a directly mapped out sensor setting and we do not yet have a relation between the change in light falling on a pixel that triggeres an event and its value. Values in [25,150] are possible. Changing this value may affect the balance between on and off events.
  uint32_t event_balance{21}; //!< Event balance - Controls the ratio between on and off events. Currently this is a directly mapped out sensor setting and we do not yet have a relation as to how exactly it influences the ratio for a given intensity step. Values in [0,31] are possible.
  uint32_t accelerometer_scale{1}; //!< [0...3] Defines operating range of accelerometer.
  uint32_t gyroscope_scale{1}; //!< [0...3] Defines operating range of gyroscope.
};


}
}


#endif //INESS_COMMON_DEVICE_SEES_SEES_CONFIG_HPP_
