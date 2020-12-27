/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Tue Feb 20 2018
 */

#ifndef LIBCAER_INESS_EVENTS_FRAME_TYPE_H_
#define LIBCAER_INESS_EVENTS_FRAME_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * Frame event data structure definition.
 * This contains the actual information on the frame (ROI, color channels,
 * color filter), several timestamps to signal start and end of capture and
 * of exposure, as well as the actual pixels, in a 16 bit normalized format.
 * The (0, 0) address is in the upper left corner, like in OpenCV/computer graphics.
 * The pixel array is laid out row by row (increasing X axis), going from
 * top to bottom (increasing Y axis).
 * Signed integers are used for fields that are to be interpreted
 * directly, for compatibility with languages that do not have
 * unsigned integer types, such as Java.
 */
#pragma pack(push, 1)
struct caer_frame_event {
	/// Event information (ROI region, color channels, color filter). First because of valid mark.
	uint32_t info;
	/// Start of Frame (SOF) timestamp.
	int32_t ts_startframe;
	/// End of Frame (EOF) timestamp.
	int32_t ts_endframe;
	/// Start of Exposure (SOE) timestamp.
	int32_t ts_startexposure;
	/// End of Exposure (EOE) timestamp.
	int32_t ts_endexposure;
	/// X axis length in pixels.
	int32_t lengthX;
	/// Y axis length in pixels.
	int32_t lengthY;
	/// X axis position (lower left offset) in pixels.
	int32_t positionX;
	/// Y axis position (lower left offset) in pixels.
	int32_t positionY;
	/// Pixel array, 16 bit unsigned integers, normalized to 16 bit depth.
	/// The pixel array is laid out row by row (increasing X axis), going
	/// from top to bottom (increasing Y axis).
	uint16_t pixels[];
};
#pragma pack(pop)//__attribute__((__packed__));

#ifdef __cplusplus
}
#endif

#endif // LIBCAER_INESS_EVENTS_FRAME_TYPE_H_