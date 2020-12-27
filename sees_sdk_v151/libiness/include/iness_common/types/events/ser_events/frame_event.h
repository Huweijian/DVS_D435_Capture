#ifndef FRAME_EVENT_H
#define FRAME_EVENT_H

#include <libcaer-iness/events/frame_type.h>
#include "iness_common/time/definitions.hpp"
#include "event_helper.h"

#undef TYPE_MASK
#include <opencv2/core.hpp>

namespace iness {

/**
 * @brief The FrameEvent class.
 * Wraps the caer frame event.
 */
#pragma pack(push, 1)
class FrameEvent
{
public:
    /**
     * @brief Get event information. Contains ROI, color channels, color filter.
     */
    uint32_t getInfo() const;

    /**
     * @brief Set event information.
     */
    void setInfo(const uint32_t &_info);

    /**
     * @brief Gets the full 64 bit timestamp.
     * @param _overflow_timestamp The timestamp overflow counter.
     */
    time::TimeUs getTimestampUs(int32_t _overflow_timestamp) const;

    /**
     * @brief Returns the width of the image.
     */
    int32_t width() const;

    /**
     * @brief Returns the height of the image.
     */
    int32_t height() const;

    /**
     * @brief Returns the image
     */
    cv::Mat image();

    /**
     * @brief Get start frame timestamp.
     */
    int32_t getTsStartFrame() const;

    /**
     * @brief Set start frame timestamp.
     */
    void setTsStartFrame(const int32_t &value);

    /**
     * @brief Get end frame timestamp.
     */
    int32_t getTsEndFrame() const;

    /**
     * @brief Set end frame timestamp.
     */
    void setTsEndFrame(const int32_t &value);

    /**
     * @brief Get timestamp of exposure start.
     */
    int32_t getTsStartExposure() const;

    /**
     * @brief Set timestamp of exposure start.
     */
    void setTsStartExposure(const int32_t &value);

    /**
     * @brief Get timestamp of exposure end.
     */
    int32_t getTsEndExposure() const;

    /**
     * @brief Set timestamp of exposure end.
     */
    void setTsEndExposure(const int32_t &value);

    /**
     * @brief Gets the number of pixels in the X axis.
     */
    int32_t getLengthX() const;

    /**
     * @brief Sets the number of pixels in the X axis.
     */
    void setLengthX(const int32_t &value);

    /**
     * @brief Gets the number of pixels in the X axis.
     */
    int32_t getLengthY() const;

    /**
     * @brief Sets the number of pixels in the X axis.
     */
    void setLengthY(const int32_t &value);

    /**
     * @brief Gets the X axis position (lower left offset) in pixels.
     */
    int32_t getPositionX() const;

    /**
     * @brief Sets the X axis position (lower left offset) in pixels.
     */
    void setPositionX(const int32_t &value);

    /**
     * @brief Gets the Y axis position (lower left offset) in pixels.
     */
    int32_t getPositionY() const;

    /**
     * @brief Sets the Y axis position (lower left offset) in pixels.
     */
    void setPositionY(const int32_t &value);

    /**
     * @brief Get the pixel values
     * @returns An OpenCV 2D matrix containing the pixel values.
     */
    cv::Mat getImage();

private:
    //! Copy constructor is private as long as it is not implemented (it would have to copy the underlying, wrapped data)
    FrameEvent(const FrameEvent&)=default;

private:
    caer_frame_event event_;
};
#pragma pack(pop)


}

#endif // FRAME_EVENT_H
