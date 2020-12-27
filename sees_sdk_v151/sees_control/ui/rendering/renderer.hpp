/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <QWidget>
#include <memory>
#include <opencv2/core.hpp>
#include <deque>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <atomic>
#include <functional>

#include "i_render_rule.hpp"
#include "iness_common/types/events/ser_events/polarity_event.h"
#include "iness_common/types/events/ser_events/event_packet.h"

/**
 * @brief The ImageView class. Has three different view modes which can be combined and are stacked in the follwing order from lowest to highest: Images, polarity events, vector fields.
 */
class Renderer : public QWidget
{
    Q_OBJECT
public:
    enum struct PolarityMixRenderModus
    {
        OVERWRITE, // overwrite color from a previous event
        MIX // mixes color of different polarity events
    };

public:
    /**
     * @brief ImageView ctor
     * @param _resX Image width in pixels. This is required for displaying polarity events.
     * @param _resY Image height in pixels. This is required for displaying polarity events.
     * @param _render_rule IRenderRule object. Determines how to color and scale pixels and vectors.
     * @param parent
     */
    explicit Renderer(int _resX, int _resY, std::shared_ptr<IRenderRule> _render_rule, QWidget *parent = 0);

    /**
     * @brief Adds new polarity event for display.
     * @param _event A polarity event.
     * @param _event_packet The packet in which the event is contained.
     */
    void updatePolarity(const iness::PolarityEvent &_event, const iness::PolarityEventPacket &_event_packet);

    /**
     * @brief Add new image data for display.
     * @param _image The image in OpenCV Mat(rix) format.
     */
    void updateImage(const cv::Mat &_image, uint64_t _time_stamp);

    /**
     * @brief Set the sensor size.
     * @param _height input height
     * @param _width  input width
     */
    void setInputSize(const unsigned int _height, const unsigned int _width);

    void startInternalImageTrigger(const int _refresh_time_in_ms = 40);

    void stopInternalImageTrigger();

    /**
     * @brief Set the render rule.
     * @param _render_rule IRenderRule object.
     */
    void setRenderRule(std::shared_ptr<IRenderRule> _render_rule);

    //! Sets the event count which is scaled to the max color intensity
    void setMaxEventCount( unsigned int _count );

    //! Sets how the different polarities shall be displayed
    void setPolarityMixRenderModus( PolarityMixRenderModus _modus );

    /**
     * @brief Get the resolution of the x axis.
     * @return
     */
    int getResolutionX();

    /**
     * @brief Get the resolution of the y axis.
     * @return
     */
    int getResolutionY();

    /**
     * @brief Clear reandering.
     */
    void clear();

    /**
     * @brief Clear rendering and set background color.
     * @param backgrond_ QColor Background color.
     */
    void clear(QColor background_);

    virtual QSize sizeHint() const;
    virtual void resizeEvent(QResizeEvent* event);

    void emitExternalTrigger();

    //! Wakes potentially blocked executions. TODO: We need to redo all of this, this should not be necessary...
    void wakeAll();

    //! Enables or disables the aps filter
    void setEnableApsFilter(bool _enable);

    //! Sets a contrast adjustment for aps frames
    void setApsContrast( float _contrast );

    //! Sets a brightness adjustment for aps frames
    void setApsBrightness( unsigned int _brightness );

    //! Set function to be called on event rate update
    void setEventRateCallback(std::function<void(float)> _callback);
    //! Set function to be called on event rate update
    void setFrameRateCallback(std::function<void(float)> _callback);

signals:
    void externalTrigger();

protected:
    /**
     * @brief Draw method, called periodically by timer.
     * @param _paint_event
     */
    void paintEvent(QPaintEvent* _paint_event) override;

private slots:
    void triggerImage();

private:

    void paintImages();

    /**
    * @brief getIncreasingSequenceFromBack Iterates backwards through input as long as element values are montonicly increasing and
    * the difference between the smallest and the largest value is below a threshold.
    * @param _input Input sequence
    * @param _max_difference Threshold of the difference between _smallest_value and _largest_value
    * @param _smallest_value First value of increasing sequence
    * @param _largest_value Last value of increasing sequence
    * @param _sequence_length_minus_one Number of elements in the increasing sequence minus one.
    */
    void getIncreasingSequenceFromBack(
            const std::deque<uint64_t>& _input,
            const uint64_t& _max_difference,
            uint64_t& _smallest_value,
            uint64_t& _largest_value,
            unsigned int& _sequence_length_minus_one);

private:
    //render sources
    QMutex image_mutex_;
    std::unique_ptr<QImage> image_;
    QMutex polarity_image_mutex_;
    std::unique_ptr<QImage> display_polarity_image_;
    std::unique_ptr<QImage> update_polarity_image_;
    cv::Mat vectors_;
    PolarityMixRenderModus polarity_render_modus_{PolarityMixRenderModus::OVERWRITE};

    unsigned int event_max_scale_{1};
    std::atomic<float> per_event_factor_{1.0f/event_max_scale_};

    //render triggering
    QTimer *internal_trigger_timer_;

    bool enable_aps_filter_{false};
    float contrast_{1.0}; // multiplicative adjustement of frame
    unsigned int brightness_{0}; // Additive offset

    // rendering rules
    std::shared_ptr<IRenderRule> current_render_rule_; // rule currently used for rendering, will be swapped with next_render_rule when new image is build
    std::shared_ptr<IRenderRule> next_render_rule_;

    // Event and frame rate computation
    static const size_t max_event_rate_queue_size_;
    static const uint64_t max_event_rate_time_window_us_;
    std::deque<uint64_t> event_rate_queue_;
    std::function<void(float)> event_rate_callback_;

    static const size_t max_frame_rate_queue_size_;
    static const uint64_t max_frame_rate_time_window_us_;
    std::deque<uint64_t> frame_rate_queue_;
    std::function<void(float)> frame_rate_callback_;

    std::atomic<bool> draw_image_ ;
    std::atomic<bool> draw_polarity_;

    QWaitCondition trigger_handled_;
    bool trigger_requested_;

    //resolution
    int resolution_x_;
    int resolution_y_;

     mutable int lastHeight_;
};

#endif // RENDERER_H
