/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef RAW_RG_RENDER_RULE_H
#define RAW_RG_RENDER_RULE_H

#include "i_render_rule.hpp"

class RawRGRenderRule : public IRenderRule
{
public:
    RawRGRenderRule();
    virtual QColor getBackgroundColor() override;
    virtual QColor getPolarityColor(const iness::PolarityEvent& _event) override;
    virtual QColor getImageColor(int x, int y, const cv::Mat& _image, float _contrast = 1.0f, unsigned int _brightness = 0) override;
    virtual QColor getVectorColor(float x, float y) override;
    virtual double getVectorScale() override;
};

#endif // RAW_RG_RENDER_RULE_H
