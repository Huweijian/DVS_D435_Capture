/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "raw_rg_render_rule.hpp"

RawRGRenderRule::RawRGRenderRule()
{

}

QColor RawRGRenderRule::getBackgroundColor()
{
    return Qt::black;
}

QColor RawRGRenderRule::getPolarityColor(const iness::PolarityEvent &_event)
{
    if( _event.getPolarity() == iness::Polarity::POS )
    {
        return Qt::green;
    }
    else{
        return Qt::red;
    }
}

QColor RawRGRenderRule::getImageColor(int x, int y, const cv::Mat& _image, float _contrast , unsigned int _brightness )
{
    int value = _image.at<uint16_t>(y,x) >> 8;
    if( _brightness!=0 )
        value+=_brightness;
    if( _contrast!=1.0f )
        value*=_contrast;
    if( value>255 )
        value=255;
    else if( value<0 )
        value=0;

    return QColor(value, value, value);
}

QColor RawRGRenderRule::getVectorColor(float, float)
{
    throw "Not implemented.";
}

double RawRGRenderRule::getVectorScale()
{
    throw "Not implemented.";
}
