/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Jonas Strubel <jonas@insightness.com>, April 2017
*/

#ifndef AEDAT_READER_AEDAT_FILE_INFO_H
#define AEDAT_READER_AEDAT_FILE_INFO_H

#include "iness_common/time/definitions.hpp"

namespace iness
{
namespace aedat
{
/**
 * @brief Info about an Aedat file.
 */
struct AedatFileInfo
{
    time::TimeUs start_time; //!< Timestamp of the first event in the file.
    time::TimeUs end_time; //!< Timestamp of the last event in the file.
};

}
}

#endif // AEDAT_READER_AEDAT_FILE_INFO_H
