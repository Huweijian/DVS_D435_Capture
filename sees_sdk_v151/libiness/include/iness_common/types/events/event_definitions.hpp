/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Jonas Strubel <jonas@insightness.com>, May 2017
*/

#ifndef INESS_COMMON_TYPES_EVENTS_EVENT_DEFINITIONS_HPP
#define INESS_COMMON_TYPES_EVENTS_EVENT_DEFINITIONS_HPP

#include <stdint.h>

namespace iness {
    /*
     *Sizes and offsets are measured in bytes DEPRECATED; DONT HARDCODE!!!
     */
    //AEDAT format DEPRECATED; DONT HARDCODE!!!
    const int AEDAT_PACKET_HEADER_SIZE = 28;

    //Event specifics DEPRECATED; DONT HARDCODE!!!
    const int BLUEFOX_FRAME_EVENT_TIMESTAMP_OFFSET = 4;
    const int POSE_EVENT_TIMESTAMP_OFFSET = 4;
    const int POSE_EVENT_SIZE = 36;

    const int MATRIX_EVENT_TIMESTAMP_OFFSET = 4;

    const int FRAME_EVENT_HEADER_LENGTH = 36;

    //Protocol
    const int TCP_HEADER_LENGTH = 20;
    const uint64_t TCP_AEDAT_3x_MAGIC_NUMBER = 0x1D378BC90B9A6658;

    //Bit shifts and masks
}

#endif // INESS_COMMON_TYPES_EVENTS_EVENT_DEFINITIONS_HPP
