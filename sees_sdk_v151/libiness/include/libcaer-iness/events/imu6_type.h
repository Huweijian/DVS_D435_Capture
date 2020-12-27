/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Tue Feb 20 2018
 */

#ifndef LIBCAER_INESS_EVENTS_IMU6_TYPE_H_
#define LIBCAER_INESS_EVENTS_IMU6_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif


/**
 * IMU 6-axes event data structure definition.
 * This contains accelerometer and gyroscope headings, plus
 * temperature.
 * The X, Y and Z axes are referred to the camera plane.
 * X increases to the right, Y going up and Z towards where
 * the lens is pointing. Rotation for the gyroscope is
 * counter-clockwise along the increasing axis, for all three axes.
 * Floats are in IEEE 754-2008 binary32 format.
 * Signed integers are used for fields that are to be interpreted
 * directly, for compatibility with languages that do not have
 * unsigned integer types, such as Java.
 */
#pragma pack(push, 1)
struct caer_imu6_event {
	/// Event information. First because of valid mark.
	uint32_t info;
	/// Event timestamp.
	int32_t timestamp;
	/// Acceleration in the X axis, measured in g (9.81m/s²).
	float accel_x;
	/// Acceleration in the Y axis, measured in g (9.81m/s²).
	float accel_y;
	/// Acceleration in the Z axis, measured in g (9.81m/s²).
	float accel_z;
	/// Rotation in the X axis, measured in °/s.
	float gyro_x;
	/// Rotation in the Y axis, measured in °/s.
	float gyro_y;
	/// Rotation in the Z axis, measured in °/s.
	float gyro_z;
	/// Temperature, measured in °C.
	float temp;
};
#pragma pack(pop)//__attribute__((__packed__));

/**
 * Type for pointer to IMU 6-axes event data structure.
 */
typedef struct caer_imu6_event *caerIMU6Event;

#ifdef __cplusplus
}
#endif

#endif // LIBCAER_INESS_EVENTS_IMU6_TYPE_H_