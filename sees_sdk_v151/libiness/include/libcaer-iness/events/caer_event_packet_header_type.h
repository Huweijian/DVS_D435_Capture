/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef LIBCAER_INESS_EVENTS_CAER_EVENT_PACKET_HEADER_TYPE_H_
#define LIBCAER_INESS_EVENTS_CAER_EVENT_PACKET_HEADER_TYPE_H_


#ifdef __cplusplus
extern "C" {
#endif


/**
 * EventPacket header data structure definition.
 * The size, also defined in CAER_EVENT_PACKET_HEADER_SIZE,
 * must always be constant. The header is common to all
 * types of event packets and is always the very first
 * member of an event packet data structure.
 * Signed integers are used for compatibility with languages that
 * do not have unsigned ones, such as Java.
 */
#pragma pack(push, 1)
struct caer_event_packet_header {
	/// Numerical type ID, unique to each event type (see 'enum caer_default_event_types').
	int16_t eventType;
	/// Numerical source ID, unique inside a process, identifies who generated the events.
	int16_t eventSource;
	/// Size of one event in bytes.
	int32_t eventSize;
	/// Offset from the start of an event, in bytes, at which the main 32 bit time-stamp can be found.
	int32_t eventTSOffset;
	/// Overflow counter for the standard 32bit event time-stamp. Used to generate the 64 bit time-stamp.
	int32_t eventTSOverflow;
	/// Maximum number of events this packet can store.
	int32_t eventCapacity;
	/// Total number of events present in this packet (valid + invalid).
	int32_t eventNumber;
	/// Total number of valid events present in this packet.
	int32_t eventValid;
};
#pragma pack(pop)//__attribute__((__packed__));

/**
 * Type for pointer to EventPacket header data structure.
 */
typedef struct caer_event_packet_header *caerEventPacketHeader;


#ifdef __cplusplus
}
#endif


#endif // LIBCAER_INESS_EVENTS_CAER_EVENT_PACKET_HEADER_TYPE_H_