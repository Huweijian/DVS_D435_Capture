/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Tue Feb 20 2018
 */

#ifndef LIBCAER_INESS_EVENTS_POLARITY_TYPE_H_
#define LIBCAER_INESS_EVENTS_POLARITY_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Polarity event data structure definition.
 * This contains the actual X/Y addresses, the polarity, as
 * well as the 32 bit event timestamp.
 * The (0, 0) address is in the upper left corner of the screen,
 * like in OpenCV/computer graphics.
 * Signed integers are used for fields that are to be interpreted
 * directly, for compatibility with languages that do not have
 * unsigned integer types, such as Java.
 */
#pragma pack(push, 1)
struct caer_polarity_event {
	/// Event data. First because of valid mark.
	uint32_t data;
	/// Event timestamp.
	int32_t timestamp;
};
#pragma pack(pop)//__attribute__((__packed__));

/**
 * Type for pointer to polarity event data structure.
 */
typedef struct caer_polarity_event *caerPolarityEvent;

#ifdef __cplusplus
}
#endif

#endif // LIBCAER_INESS_EVENTS_POLARITY_TYPE_H_