/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Tue Feb 20 2018
 */

#ifndef LIBCAER_INESS_EVENTS_PACKET_CONTAINER_TYPE_H_
#define LIBCAER_INESS_EVENTS_PACKET_CONTAINER_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "caer_event_packet_header_type.h"

/**
 * EventPacketContainer data structure definition.
 * Signed integers are used for compatibility with languages that
 * do not have unsigned ones, such as Java.
 */
#pragma pack(push, 1)
struct caer_event_packet_container {
	/// Number of different event packets contained.
	int32_t eventPacketsNumber;
	/// Array of pointers to the actual event packets.
	caerEventPacketHeader eventPackets[];
};
#pragma pack(pop)//__attribute__((__packed__));

/**
 * Type for pointer to EventPacketContainer data structure.
 */
typedef struct caer_event_packet_container *caerEventPacketContainer;


#ifdef __cplusplus
}
#endif

#endif // LIBCAER_INESS_EVENTS_PACKET_CONTAINER_TYPE_H_