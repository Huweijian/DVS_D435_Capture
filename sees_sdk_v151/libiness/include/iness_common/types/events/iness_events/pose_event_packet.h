/*
*   Copyright (C) Insightness AG, Switzerland - All Rights Reserved
*   Unauthorized copying of this file, via any medium is strictly prohibited
*   Proprietary and confidential
*   Written by Jonas Strubel <jonas@insightness.com>, May 2017
*/

#ifndef INESS_TYPES_POSE_EVENT_PACKET_H
#define INESS_TYPES_POSE_EVENT_PACKET_H

#include "iness_common/types/events/event_definitions.hpp"
#include "iness_common/types/events/iness_events/pose_event.h"
#include "iness_common/types/events/event_packet_header.h"
#include "iness_common/types/events/ser_events/templated_event_packet.hpp"

namespace iness {


typedef EventPacket<PoseEvent> PoseEventPacket;


}

#endif // INESS_TYPES_POSE_EVENT_PACKET_H
