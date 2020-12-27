/*
 *   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 *   Unauthorized copying of this file, via any medium is strictly prohibited
 *   Proprietary and confidential
 *   Written by Nathan Baumli <nathan@insightness.com>, January 2018
 */

#ifndef AEDAT_INPUT_H
#define AEDAT_INPUT_H

#include <iness_common/streaming/aedat_reader/aedat_file_info.hpp>
#include <iness_common/streaming/aedat_reader/generic_event_advertiser.hpp>
#include <iness_common/streaming/aedat_writer/aedat_source_info.hpp>
#include <iness_common/types/events/i_event_packet.hpp>

namespace iness
{

namespace device
{
class Sees; //!< Forward declaration
}

namespace aedat
{


/*!
 * The AedatInput class provides an interface for two different Aedat sources
 * - Device: Aedat packets from a SEES module.
 * - Player: Aedat packets played back from a file.
 *
 * The interface is supposed to be accessed by a single thread.
 * The registered callback functions are called in additional threads.
 */
class AedatInput
{
public:
  AedatInput();
  ~AedatInput();

  /**
   * @brief Tries to open a SEES module.
   * @return True if a device was connected and opened correctly. False if not.
   */
  bool deviceOpen();

  //! @return True if a device is open.
  bool deviceIsOpen();

  /**
   * @brief Get the device information.
   * @param _device_info Will be populated.
   * @return False if no device is open.
   */
  bool deviceGetInfo(AedatSourceInfo& _device_info);

  /**
   * @brief Get the resolution of the SEES module.
   * @param _width
   * @param _height
   * @return False if no device is open.
   */
  bool deviceGetSize(unsigned int& _width, unsigned int& _height);

  /**
   * @brief Registers a function to be called if new data is available.
   * @param _cb_func
   */
  void deviceRegisterEventPacketCallback(std::function<void(std::shared_ptr<aedat::IEventPacket>)> _cb_func);

  /**
   * @brief Starts the SEES module data readout.
   * @return False if no device is open.
   */
  bool deviceStart();

  //! Closes the SEES module.
  void deviceClose();

  /**
   * @brief Opens a file for playback. Throws an exception at failure.
   * @param _filename File to be opened.
   * @param _data_handler Callback that is called if data is available.
   * @param _source_info Is populated with the source information found in the file header. Currently
   * only single source Aedat files are supported.
   */
  void playerOpen(const std::string _filename,
      const std::shared_ptr<GenericEventAdvertiser> _data_handler,
      AedatSourceInfo& _source_info);

  //! Gets the file information.
  AedatFileInfo playerGetFileInfo();

  /**
   * @brief Registers a function to be called at a fixed rate while playing back the data.
   * @param _trigger_cp
   * @param _frame_rate_in_hz
   */
  void playerRegisterFrameTrigger(
      std::function<void()> _trigger_cp,
      const float _frame_rate_in_hz);

  /**
   * @brief Registers a function to be called on player progress.
   * @param _cb
   */
  void playerRegisterProgressCallback(std::function<void(uint64_t)> _cb);

  //! Close the file playback.
  void playerClose();

  //! Start or continue to play data.
  void playerPlay();

  //! Pause playing data.
  void playerPause();

  //! Reset the player to the start of the file and pause.
  void playerStop();

  /**
   * @brief Jump one frame.
   * @param _direction_forward True is forward, false is backward.
   */
  void playerStep(const bool _direction_forward);

  //! Activate loop mode (automatically restart if file end is reached).
  void playerLoop(const bool _loop_on);

  /**
   * @brief Set the playback speed.
   * @param _realtime_factor 1.0 is realtime, 2.0 is twice faster than and 0.5 is half realtime.
   */
  void playerSetSpeed(const double _realtime_factor);

  /**
   * @brief Publish a a fixed number of polarity events per second.
   * @param _events_per_second If zero the player switches back to normal playback mode.
   */
  void playerSetEventRate(const unsigned int _events_per_second);

  /**
   * @brief Publish a fixed number of polarity events per frame.
   * @param _events_per_frame If zero the player switches back to normal playback mode.
   */
  void playerSetEventsPerFrame(const unsigned int _events_per_frame);

  /**
   * @brief Jump to a location in the file.
   * @param _playback_time Packet timestamp in the file.
   */
  void playerSetPlaybackTime(const uint64_t _playback_time);

public:
  //! This will be removed in a future release to protect device access.
  std::weak_ptr<iness::device::Sees> deviceGet();

private:
  struct Implementation; //!< Forward Declaration
  std::unique_ptr<Implementation> impl_; //!< Pointer to implementation
};

}
}
#endif // AEDAT_INPUT_H
