/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef I_COLOR_RULE_H
#define I_COLOR_RULE_H

#include <QColor>
#include <opencv2/core.hpp>

#include "iness_common/types/events/ser_events/polarity_event.h"

/**
 * @brief The IRenderRule interface. Provides interfaces to define custom coloring schemes.
 */
class IRenderRule
{
public:
    /**
     * @brief ~IRenderRule ctor
     */
    virtual ~IRenderRule(){}

    /**
     * @brief Translate backgroudn to color.
     * @return A color.
     */
    virtual QColor getBackgroundColor() = 0;

    /**
     * @brief Translate polarity to color.
     * @param _event A polarity event.
     * @return A color.
     */
    virtual QColor getPolarityColor(const iness::PolarityEvent& _event) = 0;

    /**
     * @brief Translate image pixel value to color.
     * @param x X-Coordinate of the current pixel.
     * @param y Y-Coordinate of the current pixel.
     * @param _image The OpenCV image matrix.
     * @param _contrast Adjusts the contrast
     * @param _brightness Adjusts the brightness
     * @return  A color.
     */
    virtual QColor getImageColor(int x, int y, const cv::Mat& _image, float _contrast = 1.0f, unsigned int _brightness = 0) = 0;

    /**
     * @brief Translate vector properties to color.
     * @param x X component of vector.
     * @param y Y component of vector.
     * @return A color.
     */
    virtual QColor getVectorColor(float x, float y) = 0;

    /**
     * @brief Scaling factor for vector length display.
     * @return A scaling factor.
     */
    virtual double getVectorScale() = 0;
};

#endif // I_COLOR_RULE_H
