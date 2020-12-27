#ifndef IMU6_EVENT_H
#define IMU6_EVENT_H

#include <stdint.h>
#include <libcaer-iness/events/imu6_type.h>
#include "event_helper.h"
#include "iness_common/types/generated/aedat_type_generated.h"

namespace iness {

#pragma pack(push, 1)
class Imu6Event
{
public:
    Imu6Event() = default;

    //! Get the AedatType
    static iness::serialization::AedatType aedatType();

    //! Serialize event to buffer.
    void serialize(char *_output_buffer) const;

    /**
     * @brief Gets the event information.
     */
    uint32_t getInfo() const;

    /**
     * @brief Sets the event information.
     * @param value The raw information value.
     */
    void setInfo(const uint32_t &value);

    /**
     * @brief Get the full 64 bit timestamp which includes the overflow counter.
     * @param _overflow_timestamp The timestamp overflow counter.
     * @return
     */
    time::TimeUs getTimestampUs(int32_t _overflow_timestamp) const;

    /**
     * @brief Set the 32 bit timestamp
     * @param _timestamp The 32 bit timestamp
     */
    void setTimestamp(int32_t _timestamp);

    /**
     * @brief Gets the acceleration in the X axis, measured in g (9.81m/s²).
     */
    float getAccelerationX() const;

    /**
     * @brief Sets the acceleration in the X axis, measured in g (9.81m/s²).
     */
    void setAccelerationX(float value);

    /**
     * @brief Gets the acceleration in the Y axis, measured in g (9.81m/s²).
     * @return
     */
    float getAccelerationY() const;

    /**
     * @brief Sets the acceleration in the Y axis, measured in g (9.81m/s²).
     */
    void setAccelerationY(float value);

    /**
     * @brief Gets the acceleration in the Z axis, measured in g (9.81m/s²).
     * @return
     */
    float getAccelerationZ() const;

    /**
     * @brief Sets the acceleration in the Z axis, measured in g (9.81m/s²).
     */
    void setAccelerationZ(float value);

    /**
     * @brief Gets the rotation around the X axis, measured in °/s.
     */
    float getGyroX() const;

    /**
     * @brief Sets the rotation around the X axis, measured in °/s.
     */
    void setGyroX(float value);

    /**
     * @brief Gets the rotation around the Y axis, measured in °/s.
     */
    float getGyroY() const;

    /**
     * @brief Sets the rotation around the Y axis, measured in °/s.
     */
    void setGyroY(float value);

    /**
     * @brief Gets the rotation around the Z axis, measured in °/s.
     */
    float getGyroZ() const;

    /**
     * @brief Sets the rotation around the Z axis, measured in °/s.
     */
    void setGyroZ(float value);

    /**
     * @brief Get temperature in °C
     */
    float getTemperature() const;

    /**
     * @brief Set temperature in °C
     */
    void setTemperature(float value);

public:
    static const unsigned int ts_offset = sizeof(uint32_t); //!< size of info_

private:
    caer_imu6_event event_;
};
#pragma pack(pop)

}

#endif // IMU6_EVENT_H

