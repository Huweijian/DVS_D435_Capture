/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Stefan Isler <stefan@insightness.com>
 * Thu Jun 14 2018
 */

#ifndef INESS_COMMON_STREAMING_AEDAT_READER_SIMPLE_FILE_READER
#define INESS_COMMON_STREAMING_AEDAT_READER_SIMPLE_FILE_READER

#include <string>
#include <memory>
#include "iness_common/streaming/aedat_writer/aedat_source_info.hpp"
#include "iness_common/types/events/i_event_packet.hpp"

namespace iness
{
namespace aedat
{

//! Basic file reader that allows to iterate through aedat packets reading out event packets one by one
class SimpleFileReader
{
public:
  /*! Constructor
   * @param _file File to be opened, parsed and read
   */
  SimpleFileReader( std::string _file );

  //! Destructor
  ~SimpleFileReader();

  /*! Returns information about what sources were
   *  used to record the aedat file.
   */
  std::vector<AedatSourceInfo> getSourceInfo();

  /*! Returns the next packet, a nullptr if none left.
   */
  IEventPacket::Ptr next();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}
}

#endif // INESS_COMMON_STREAMING_AEDAT_READER_SIMPLE_FILE_READER