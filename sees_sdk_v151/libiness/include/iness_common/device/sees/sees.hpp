/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 *            Nathan Baumli <nathan@insightness.com>
 *            Jonas Strubel
 * Wed Dec 13 2017
 */

#ifndef INESS_COMMON_DEVICE_SEES_DRIVER_H_
#define INESS_COMMON_DEVICE_SEES_DRIVER_H_

#include <vector>
#include <functional>
#include <string>
#include <atomic>
#include <mutex>

#include "iness_common/device/sees_base.hpp"
#include "iness_common/device/sees/sees_config.hpp"
#include "iness_common/device/sees/device_requirements.hpp"

#include "iness_common/types/events/i_event_packet.hpp"
#include "iness_common/types/events/ser_events/event_packet.h"
#include "iness_common/types/events/event_packet_header.h"
#include "iness_common/types/events/ser_events/polarity_event.h"
#include "iness_common/types/events/ser_events/frame_event.h"
#include "iness_common/types/events/ser_events/imu6_event.h"
#include "iness_common/types/events/ser_events/special_event.h"
#include "iness_common/types/sees_container.hpp"
#include "iness_common/types/events/ser_events/event_packet_container_ptr.hpp"

#include "iness_common/util/parameter_tree/provider.hpp"


namespace iness{

namespace util{ 
  class AutoExposureBase; //< Forward declaration of the auto exposure controller
}

namespace device{

class SerMetricsManager; //< Forward declaration of the sensor metrics manager
class DeviceWrapper; //< Forward declaration of the device wrapper

//! Main interface to Insightness silicon eye event sensor (SEES/SEEM) devices
/*! The Sees class serves as C++ interface to Insightness silicon eye event sensor (SEES) devices. Each Sees instance can connect to
 *  one plugged in device at a time and be used to both configure the device and read out its data (polarity events, frames and IMU).
 *  Supported SEES device types are currently Seem0a, Seem0b and Seem1. Data is read out by registering so-called callback functions
 *  which will be called when a certain data type was received. If you have more than one device connected and/or want to ensure that
 *  only a specific module and/or type should be opened, you can use the DeviceRequirements struct when creating the Sees device
 *  instance.*/
class Sees: public SeesBase, public ptree::Provider
{
public:
  typedef SeesConfig Config;
    
public:
  /*! The constructor attempts to connect to a device that fulfills the given requirements. Loads the default settings.
   *
   * @param _requirements The requirements struct can be used to define the type of device that is supposed to be opened, as the Sees driver
   *                      supports several types (currently Seem0a, Seem0b and Seem1) and multiple of each could be connected to a computer.
   *                      The first device fitting all specified requirements in the struct is opened.
   */
  Sees( DeviceRequirements _requirements = DeviceRequirements() );

  /*! The constructor attempts to connect to a device that fulfills the given requirements. Additionally loads the settings from the argument 
   * file or creates the argument file and writes the default settings to it, in case the file does not exist.
   * 
   * @param _sees_config_file The YAML config file
   * @param _requirements The requirements struct can be used to define the type of device that is supposed to be opened, as the Sees driver
   *                      supports several types (currently Seem0a, Seem0b and Seem1) and multiple of each could be connected to a computer.
   *                      The first device fitting all specified requirements in the struct is opened.
   */
  Sees(const std::string& _sees_config_file, DeviceRequirements _requirements = DeviceRequirements() );
  ~Sees();

  //! Returns if connection to a device was successful and whether it is (still) ready to be used.
  bool isOk() const;

  /*! @brief Returns the device id, which is an id unique for every opened device.
   *
   *  It is the same identifier as will be used as a source id within the transmitted
   *  aedat packets.
   *  @return -1 if no device id is available, the device id otherwise
   */
  virtual int deviceId() const;

  //! Returns the name of the device module type.
  std::string deviceTypeName() const;

  //! Returns the serial number of the device.
  std::string serialNumber() const;

  //! Attempts to start receiving data.
  /*! @return Flag whether sensor is successfully started.
   */
  bool start();

  //! Stop receiving data.
  void stop();

  //! Returns true if data acquisition is running (the start() function call succeeded and stop() has not been called)
  bool isAcquiringData() const;

  //! Returns the polarity event "frame" width
  unsigned int dvsWidth() const;

  //! Returns the polarity event "frame" height
  unsigned int dvsHeight() const;

  //! Returns the aps frame width
  unsigned int apsWidth() const;

  //! Returns the aps frame height
  unsigned int apsHeight() const;

  //! Loads high-level configuration
  bool loadConfiguration( SeesConfig& _config );

  /**
   * @brief Attempts to load configuration from file.
   * @param _sees_config_file The YAML config file for the device.
   */
  bool loadConfiguration(const std::string& _sees_config_file);

  /*! @brief Writes the configuration to a file in yaml format. */
  bool writeConfigurationToFile( const std::string& _target_file );

  //! Set the APS Enabled Flag
  bool setApsEnabled(bool _flag);

  //! Returns the APS Enabled Flag
  bool getApsEnabled() const;

  //! Set the IMU Enabled Flag
  bool setImuEnabled(bool _flag);

  //! Returns the IMU Enabled Flag
  bool getImuEnabled() const;

  //! Set the DVS Enabled Flag
  bool setDvsEnabled(bool _flag);

  //! Returns the DVS Enabled Flag
  bool getDvsEnabled() const;

  //! Reset the device time to zero
  bool resetDeviceTime();

  //! Set the event threshold
  /*! Controls the sensitivity of the device to changes. Currently this is a directly mapped out sensor setting 
   *  and we do not yet have a relation between the change in light falling on a pixel that triggeres an event
   *  and its value. Values in [25,150] are possible. Changing this value may affect the balance between on and
   *  off events.
   */
  bool setEventThreshold(uint32_t _event_threshold);

  //! Returns event threshold
  uint32_t getEventThreshold() const;

  //! Set the event balance (between on and off events)
  /*! Event balance - Controls the ratio between on and off events. Currently this is a directly mapped out sensor
   *  setting and we do not yet have a relation as to how exactly it influences the ratio for a given intensity step.
   *  Values in [0,31] are possible.
   */
  bool setEventBalance(uint32_t _event_threshold);

  //! Returns the event balance (between on and off events)
  uint32_t getEventBalance() const;

  //! Enable or disable the auto-exposure.
  /*! Note that due to the inner working of the device, this only works with enabled polarity (DVS) events.
   */
  bool setAutoExposureEnabled(bool _flag);

  //! Checks if auto-exposure is enabled
  bool isAutoExposureEnabled() const;

  /*! @brief Sets the median frame brightness to which the auto exposure algorithm controls the exposure time.
   *  
   *  The given brightness value is relative to the range of the image, i.e. (0.0,1.0). Currently, SEES frames are
   *  encoded using a 16bit array, though the actual resolution is lower for Seem0a, Seem0b and Seem1 and not the
   *  full numeric range is used. The currently included auto exposure algorithm calculates the median brightness
   *  of every frame and adjusts the exposure time such that (in theory) it would have equaled the defined reference
   *  value if captured with the new exposure time. If not set by the user, the median reference is set to a
   *  module-specific default value. Given the characteristics of our devices, such as the limited dynamic range of
   *  the APS frame, the optimal value likely needs to be tuned per application and environment.
   * 
   *  @param _median_brightness Reference value for the auto exposure algorithm (0...1).
   */ 
  void setAutoExposureMedianBrightness( float _median_brightness );

  //! Returns the currently set median brightness of the auto exposure algorithm.
  float getAutoExposureMedianBrightness() const;

  //! Set the Exposure Time (us)
  bool setExposure(uint32_t _exposure_us);

  //! Returns the Exposure Time (us)
  uint32_t getExposure() const;

  //! Set the Framerate (hz)
  bool setFrameRate(uint32_t _frame_rate_hz);

  //! Returns the Framerate (hz)
  uint32_t getFrameRate() const;

  //! Set the Gyro Scale (i.e. Operating Range)
  bool setGyroScale(uint32_t _gyro_scale);

  //! Returns the Gyro Scale (i.e. Operating Range)
  uint32_t getGyroScale() const;

  //! Set the Accelerometer Scale (i.e. Operating Range)
  bool setAccelerometerScale(uint32_t _accelerometer_scale);

  //! Returns the Accelerometer Scale (i.e. Operating Range)
  uint32_t getAccelerometerScale() const;

  //! Registers a container level callback.
  /*!
   * The data is guaranteed to be available until the last copy of the SeesContainerPtr is destroyed.
   */
  virtual void registerCallback( std::function<void(SeesContainerPtr)> _func ) override;

  /*! @brief Register a callback that will be called on every packet.
   *
   *  The IEventPacket type offers functionality to check which data type it actually contains. It is
   *  guaranteed that the data is available until the last shared_ptr to the IEventPacket runs out
   *  of scope.
   * 
   *  @param _cb_func Function to which packets will be passed.
   */
  virtual void registerCallback( std::function<void(std::shared_ptr<aedat::IEventPacket>)> _cb_func ) override;

  //! Register new polarity event callback function. Gets called per polarity event.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(PolarityEvent&, PolarityEventPacket&)> _func ) override;

  //! Register new polarity event packet callback function. Gets called per polarity event packet.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(PolarityEventPacket&)> _func ) override;

  //! Register new frame event callback function. Gets called per polarity event.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(FrameEvent&,FrameEventPacket&)> _func ) override;

  //! Register new frame event packet callback function. Gets called per polarity event packet.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(FrameEventPacket&)> _func ) override;

  //! Register new IMU6 event callback function. Gets called per polarity event.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(Imu6Event&,EventPacket<Imu6Event>&)> _func ) override;

  //! Register new IMU6 event packet callback function. Gets called per polarity event packet.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(EventPacket<Imu6Event>&)> _func ) override;

  //! Register new special event callback function. Gets called per polarity event.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(SpecialEvent&,EventPacket<SpecialEvent>&)> _func ) override;

  //! Register new special event packet callback function. Gets called per polarity event packet.
  /** Attention: The data within the packet is only guaranteed to be valid until the callback function returns.
   */
  virtual void registerCallback( std::function<void(EventPacket<SpecialEvent>&)> _func ) override;

  /*! @brief Getter for metrics manager.
   * @return Shared pointer to metrics manager instance.
   */
  std::shared_ptr<iness::device::SerMetricsManager> getMetricsManager();

  //! Returns the parameter tree.
  /*! The parameter tree is a generic representation of the devices' parameter structure, which can be used
   *  to read from and write to those parameters.
   */
  virtual ptree::Node::Ptr parameterTree() override;

  //Methods
private:

  //! Handles USB connection loss.
  static void shutdownNotify(void*);

  //! Handles data processing upon arrival of new event containers.
  static void dataNotifyIncrease(void*);
  
  //! Calls registered callbacks to inform listeners about new data.
  void handleGenericEvents(SharedEventPacketContainerPtr _container);

  //! Calls registered callbacks to inform listeners about new data.
  void handlePolarityEvents(SharedEventPacketContainerPtr container);

  //! Calls registered callbacks to inform listeners about new data.
  void handleImu6Events(SharedEventPacketContainerPtr container);

  //! Calls registered callbacks to inform listeners about new data.
  void handleFrameEvents(SharedEventPacketContainerPtr container);

  //! Calls registered callbacks to inform listeners about new data.
  void handleSpecialEvents(SharedEventPacketContainerPtr container);

  //! Invoke all registered event callback and event packet callback functions to process the container data.
  void processEventPackets(SharedEventPacketContainerPtr _container);

  //! Invoke all registered container callback functions to process the container data.
  void processContainer(SharedEventPacketContainerPtr _container);

  //Member variables
private:
  std::shared_ptr<DeviceWrapper> device_; //!< Pointer to the opened device

  ptree::Node::Ptr config_tree_; //!<  tree for the Sees class.

  std::shared_ptr<iness::device::SerMetricsManager> metrics_manager_;

  std::shared_ptr<iness::util::AutoExposureBase> auto_exposure_;

  std::mutex cb_mut_;
  std::vector<std::function<void(PolarityEvent&, PolarityEventPacket&)>> polarity_event_callbacks_;
  std::vector<std::function<void(PolarityEventPacket&)>> polarity_event_packet_callbacks_;
  
  std::vector<std::function<void(std::shared_ptr<aedat::IEventPacket>)>> event_packet_callbacks_;

  std::vector<std::function<void(FrameEvent&, FrameEventPacket&)>> frame_event_callbacks_;
  std::vector<std::function<void(FrameEventPacket&)>> frame_event_packet_callbacks_;

  std::vector<std::function<void(Imu6Event&, EventPacket<Imu6Event>&)>> imu6_event_callbacks_;
  std::vector<std::function<void(EventPacket<Imu6Event>&)>> imu6_event_packet_callbacks_;

  std::vector<std::function<void(SpecialEvent&, EventPacket<SpecialEvent>&)>> special_event_callbacks_;
  std::vector<std::function<void(EventPacket<SpecialEvent>&)>> special_event_packet_callbacks_;

  std::string sees_config_file_;

  std::vector< std::function<void(SeesContainerPtr)> > container_callbacks_;
};

}
}

#endif // INESS_COMMON_DEVICE_SEES_DRIVER_H_

