/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Wed Dec 13 2017
 */

#ifndef INESS_COMMON_DEVICE_DEVICE_REQUIREMENTS_HPP_
#define INESS_COMMON_DEVICE_DEVICE_REQUIREMENTS_HPP_

#include <map>
#include <string>
#include <memory>
#include <functional>

namespace iness
{
namespace device
{

//! Suported device types
typedef int DeviceType;
static constexpr DeviceType UNDEFINED_DEVICE = -1;

//! Defines requirements for the device that is to be loaded (-1 indicates no requirement for a given entry)
/*! Our Sees device interface class can connect to multiple types and will connect to the first one it finds
 *  and supports. In order to restrict it to a specific device or device type, requirements can be specified
 *  through this struct. Our Sees device interface class will then connect to the first device fulfilling
 *  all specified requirements and to none if none fits all.
 */
struct DeviceRequirements
{
  DeviceType device_type_id{UNDEFINED_DEVICE}; //!< Device type identifier, -1 means no specific requirement
  std::string device_type_name{""}; //!< Alternative to device type identifier: use the device types name. Currently supported: Seem0a, Seem0b, Seem1
  std::string serial_nr{""}; //!< Serial number requirement.
};

}
}


#endif // INESS_COMMON_DEVICE_DEVICE_REQUIREMENTS_HPP_
