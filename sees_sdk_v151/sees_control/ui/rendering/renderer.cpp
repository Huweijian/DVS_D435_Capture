/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "renderer.hpp"

#include <math.h>

#include <QPen>
#include <QPainter>
#include <QColor>
#include <QTimer>

constexpr int line_thickness = 1;

const QRgb polarity_bkg_overlay = Qt::transparent;
const QColor polarity_bkg = Qt::black;

const size_t Renderer::max_event_rate_queue_size_ = 90000;
const uint64_t Renderer::max_event_rate_time_window_us_ = 1000;

const size_t Renderer::max_frame_rate_queue_size_ = 10;
const uint64_t Renderer::max_frame_rate_time_window_us_ = 1500000;

Renderer::Renderer(int _resX, int _resY, std::shared_ptr<IRenderRule> _render_rule, QWidget *parent)
    : QWidget (parent), lastHeight_(0)
{
    draw_image_ = false;
    draw_polarity_ = false;
    trigger_requested_ = false;

    current_render_rule_ = _render_rule;
    next_render_rule_ = _render_rule;
    setInputSize(_resY,_resX);

    //Set widget background color
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    QSizePolicy policy(this->sizePolicy());
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    this->setSizePolicy(policy);

    //Start render timer
    internal_trigger_timer_ = new QTimer(this);
    connect(internal_trigger_timer_, SIGNAL(timeout()), this, SLOT(triggerImage()));
    connect(this, SIGNAL(externalTrigger()), this, SLOT(triggerImage()));
}

void Renderer::updatePolarity(const iness::PolarityEvent& _event, const iness::PolarityEventPacket& _event_packet)
{
    polarity_image_mutex_.lock();

    if(trigger_requested_)
    {
        trigger_handled_.wait(&polarity_image_mutex_);
    }

    QRgb *pixel = (QRgb*)update_polarity_image_->scanLine(_event.getY());
    pixel = &pixel[_event.getX()];

    QColor c1;
    QColor c2 = current_render_rule_->getPolarityColor(_event);

    if( polarity_render_modus_==PolarityMixRenderModus::MIX )
    {
        c1 = *pixel;

        uint16_t red, green, blue;
        if( event_max_scale_==1 )
        {
            red = std::max(c1.red(),c2.red());
            green = std::max(c1.green(),c2.green());
            blue = std::max(c1.blue(),c2.blue());
        }
        else
        {
            red = std::min<uint16_t>(255, per_event_factor_*c2.red() + c1.red() );
            green = std::min<uint16_t>(255, per_event_factor_*c2.green() + c1.green() );
            blue = std::min<uint16_t>(255, per_event_factor_*c2.blue() + c1.blue() );
        }
        *pixel = QColor( red, green, blue).rgb();
    }
    else
        *pixel = c2.rgb();

    draw_polarity_ = true;

    //update event rate estimation
    if(event_rate_queue_.size() >= max_event_rate_queue_size_)
    {
        event_rate_queue_.pop_front();
    }
    event_rate_queue_.push_back(_event.getTimestampUs(_event_packet.tsOverflowCount()));

    polarity_image_mutex_.unlock();
}

void Renderer::updateImage(const cv::Mat &_image, uint64_t _time_stamp)
{   
    QMutexLocker image_locker(&image_mutex_);

    //resize image to fit incoming format
    if(image_->width() != _image.cols || image_->height() != _image.rows)
        image_.reset(new QImage(_image.cols, _image.rows, QImage::Format_ARGB32));

    for(int x = 0; x < _image.cols; x++)
    {
        for(int y = 0; y < _image.rows; y++)
        {
            QRgb *pixel = (QRgb*)image_->scanLine(y);
            pixel = &pixel[x];
            if(enable_aps_filter_)
                *pixel = current_render_rule_->getImageColor(x,y,_image, contrast_, brightness_).rgb();
            else
                *pixel = current_render_rule_->getImageColor(x,y,_image).rgb();
        }
    }
    draw_image_ = true;

    // update frame rate estimate
    if(frame_rate_queue_.size() >= max_frame_rate_queue_size_)
    {
        frame_rate_queue_.pop_front();
    }
    frame_rate_queue_.push_back(_time_stamp);
}

void Renderer::setInputSize(const unsigned int _height, const unsigned int _width)
{
    QMutexLocker image_locker(&image_mutex_);
    QMutexLocker polarity_locker(&polarity_image_mutex_);

    resolution_x_ = _width;
    resolution_y_ = _height;

    //Init images
    image_.reset(new QImage(resolution_x_, resolution_y_, QImage::Format_ARGB32));
    display_polarity_image_.reset(new QImage(resolution_x_, resolution_y_, QImage::Format_ARGB32));
    update_polarity_image_.reset(new QImage(resolution_x_, resolution_y_, QImage::Format_ARGB32));

    image_->fill(current_render_rule_.get()->getBackgroundColor());
    display_polarity_image_->fill(polarity_bkg_overlay);
    update_polarity_image_->fill(polarity_bkg_overlay);
    //draw_vectors_ = false;
    draw_polarity_ = false;

    updateGeometry();
}

void Renderer::startInternalImageTrigger(const int _refresh_time_in_ms)
{
    internal_trigger_timer_->start(_refresh_time_in_ms);
}

void Renderer::stopInternalImageTrigger()
{
    internal_trigger_timer_->stop();
}

void Renderer::emitExternalTrigger()
{
    polarity_image_mutex_.lock();
    trigger_requested_ = true;
    polarity_image_mutex_.unlock();

    emit externalTrigger();
}

void Renderer::wakeAll()
{
    trigger_requested_ = false;
    trigger_handled_.wakeAll();
}

void Renderer::setEnableApsFilter(bool _enable)
{
    enable_aps_filter_ = _enable;
}

void Renderer::setApsContrast( float _contrast )
{
    contrast_ = _contrast;
}

void Renderer::setApsBrightness( unsigned int _brightness )
{
    brightness_ = _brightness;
}

void Renderer::setEventRateCallback(std::function<void(float)> _callback)
{
    event_rate_callback_ = _callback;
}

void Renderer::setFrameRateCallback(std::function<void(float)> _callback)
{
    frame_rate_callback_ = _callback;
}

void Renderer::triggerImage()
{
    QMutexLocker polarity_locker(&polarity_image_mutex_);

    // Swap pointers.
    display_polarity_image_.swap(update_polarity_image_);
    current_render_rule_.swap(next_render_rule_);
    next_render_rule_ = current_render_rule_;

    // Clear polarity image.
    if(draw_image_)
        update_polarity_image_->fill(polarity_bkg_overlay); //if overlay to camera image, make background transparent
    else
        update_polarity_image_->fill(polarity_bkg);

    trigger_requested_ = false;
    trigger_handled_.wakeAll();
    emit update();
}

void Renderer::setRenderRule(std::shared_ptr<IRenderRule> _render_rule)
{
    QMutexLocker polarity_locker(&polarity_image_mutex_);
    next_render_rule_ = _render_rule;
}

void Renderer::setPolarityMixRenderModus( PolarityMixRenderModus _modus )
{
    polarity_render_modus_ = _modus;
}

void Renderer::setMaxEventCount( unsigned int _count )
{
    event_max_scale_ = _count;
    per_event_factor_ = 1.0f/_count;
}

int Renderer::getResolutionX()
{
    return resolution_x_;
}

int Renderer::getResolutionY()
{
    return resolution_y_;
}

void Renderer::clear()
{
    QMutexLocker image_locker(&image_mutex_);
    QMutexLocker polarity_locker(&polarity_image_mutex_);

    image_->fill(next_render_rule_->getBackgroundColor());
    display_polarity_image_->fill(polarity_bkg_overlay);
    update_polarity_image_->fill(polarity_bkg_overlay);

    draw_polarity_ = false;
}

void Renderer::clear(QColor background_)
{
    QMutexLocker image_locker(&image_mutex_);
    QMutexLocker polarity_locker(&polarity_image_mutex_);

    image_->fill(background_);
    display_polarity_image_->fill(polarity_bkg_overlay);
    update_polarity_image_->fill(polarity_bkg_overlay);
    draw_polarity_ = false;
}

QSize Renderer::sizeHint() const
{
    QSize s = size();
    lastHeight_ = s.height();
    s.setWidth((s.height()*resolution_x_)/resolution_y_);
    s.setHeight(QWidget::sizeHint().height());
    return s;
}

void Renderer::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (lastHeight_!=height())
    {
        updateGeometry();
    }
}

void Renderer::paintEvent(QPaintEvent*)
{
    paintImages();
}

void Renderer::paintImages()
{
    QMutexLocker image_locker(&image_mutex_);
    QMutexLocker polarity_locker(&polarity_image_mutex_);

    QPainter painter;
    painter.begin(this);
    QRect rect = this->rect();

    if( polarity_render_modus_==PolarityMixRenderModus::MIX )
        painter.setCompositionMode(QPainter::CompositionMode::CompositionMode_Plus);

    //Draw camera image
    if(draw_image_)
    {
        painter.drawImage(rect,*image_);
    }

    //Overlay polarity image
    painter.drawImage(rect,*display_polarity_image_);

    painter.end();

    // update event rate
    float mega_events_per_second = 0.0;
    if(!event_rate_queue_.empty())
    {
        uint64_t smallest_stamp, largest_stamp;
        unsigned int increments;
        getIncreasingSequenceFromBack(event_rate_queue_, max_event_rate_time_window_us_, smallest_stamp, largest_stamp, increments);

        auto delta_t = largest_stamp - smallest_stamp;
        if(delta_t > 0)
        {
            mega_events_per_second = (double) increments / (double) delta_t;
        }
    }

    if(event_rate_callback_)
    {
        event_rate_callback_(mega_events_per_second);
    }

    // update frame rate
    float frames_per_second = 0.0;
    if(!frame_rate_queue_.empty())
    {
        uint64_t smallest_stamp, largest_stamp;
        unsigned int increments;
        getIncreasingSequenceFromBack(frame_rate_queue_, max_frame_rate_time_window_us_, smallest_stamp, largest_stamp, increments);

        auto delta_t = largest_stamp - smallest_stamp;
        if(delta_t > 0)
        {
            frames_per_second = (double) increments / ((double) delta_t/(double) 1000000);
        }
    }

    if(frame_rate_callback_)
    {
        frame_rate_callback_(frames_per_second);
    }

}

void Renderer::getIncreasingSequenceFromBack(
        const std::deque<uint64_t>& _input,
        const uint64_t& _max_difference,
        uint64_t& _smallest_value,
        uint64_t& _largest_value,
        unsigned int& _sequence_length_minus_one)
{
    auto reverse_iterator = _input.rbegin();
    _largest_value = *reverse_iterator;
    _smallest_value = _largest_value;
    _sequence_length_minus_one = 0;

    for (reverse_iterator = ++_input.rbegin(); reverse_iterator!= _input.rend(); ++reverse_iterator)
    {
        // check if monotonic or difference to large
        if(_smallest_value <  *reverse_iterator || (_largest_value - *reverse_iterator) > _max_difference)
        {
            break;
        }
        else
        {
            _sequence_length_minus_one++;
            _smallest_value = *reverse_iterator;
        }
    }
}
