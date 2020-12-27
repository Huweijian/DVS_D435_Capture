/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Tue Feb 20 2018
 */

#ifndef LIBCAER_INESS_EVENTS_SPECIAL_TYPE_H_
#define LIBCAER_INESS_EVENTS_SPECIAL_TYPE_H_


#ifdef __cplusplus
extern "C" {
#endif

/**
 * List of all special event type identifiers.
 * Used to interpret the special event type field.
 */
enum caer_special_event_types {
	LIBCAER_SPECIAL_EVENT_TYPE_TIMESTAMP_WRAP = 0,              //!< A 32 bit timestamp wrap occurred.
	LIBCAER_SPECIAL_EVENT_TYPE_TIMESTAMP_RESET = 1,             //!< A timestamp reset occurred.
	LIBCAER_SPECIAL_EVENT_TYPE_EXTERNAL_INPUT_RISING_EDGE = 2,  //!< A rising edge was detected (External Input module on device).
	LIBCAER_SPECIAL_EVENT_TYPE_EXTERNAL_INPUT_FALLING_EDGE = 3, //!< A falling edge was detected (External Input module on device).
	LIBCAER_SPECIAL_EVENT_TYPE_EXTERNAL_INPUT_PULSE = 4,        //!< A pulse was detected (External Input module on device).
	LIBCAER_SPECIAL_EVENT_TYPE_DVS_ROW_ONLY = 5,    //!< A DVS row-only event was detected (a row address without any following column addresses).
	LIBCAER_SPECIAL_EVENT_TYPE_TOO_MANY_EVENTS = 18,
	LIBCAER_SPECIAL_EVENT_TYPE_EXTERNAL_INPUT_RISING_EDGE_COUNT = 48,  //!< Counter for rising edge input. Can count to 63 (6bit).
    LIBCAER_SPECIAL_EVENT_TYPE_EXTERNAL_INPUT_FALLING_EDGE_COUNT = 49, //!< Counter for falling edge input. Can count to 63 (6bit).
    LIBCAER_SPECIAL_EVENT_TYPE_DATA_DROPPED = 50 //!< Data in one of the hardware FIFOs was dropped. Bit encoding: |0|TIME|EXT|IMU|APS|DVS|.
};


/**
 * Special event data structure definition.
 * This contains the actual data, as well as the 32 bit event timestamp.
 * Signed integers are used for fields that are to be interpreted
 * directly, for compatibility with languages that do not have
 * unsigned integer types, such as Java.
 */
#pragma pack(push, 1)
struct caer_special_event {
	/// Event data. First because of valid mark.
	uint32_t data;
	/// Event timestamp.
	int32_t timestamp;
};
#pragma pack(pop)//__attribute__((__packed__));

/**
 * Type for pointer to special event data structure.
 */
typedef struct caer_special_event *caerSpecialEvent;


#ifdef __cplusplus
}
#endif

#endif // LIBCAER_INESS_EVENTS_SPECIAL_TYPE_H_