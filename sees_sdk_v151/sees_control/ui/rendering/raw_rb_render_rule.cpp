/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "raw_rb_render_rule.hpp"

RawRBRenderRule::RawRBRenderRule()
{

}

QColor RawRBRenderRule::getBackgroundColor()
{
    return Qt::black;
}

QColor RawRBRenderRule::getPolarityColor(const iness::PolarityEvent &_event)
{
    if( _event.getPolarity() == iness::Polarity::POS )
    {
        return QColor(0,0,255);
    }
    else{
        return QColor(255,0,0);
    }
}

QColor RawRBRenderRule::getImageColor(int x, int y, const cv::Mat& _image, float _contrast , unsigned int _brightness )
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

QColor RawRBRenderRule::getVectorColor(float, float)
{
    throw "Not implemented.";
}

double RawRBRenderRule::getVectorScale()
{
    throw "Not implemented.";
}
