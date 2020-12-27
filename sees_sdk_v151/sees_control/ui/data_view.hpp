/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef DATA_VIEW_H
#define DATA_VIEW_H

#include <memory>
#include <QWidget>
#include <QVBoxLayout>

#include "ui/rendering/renderer.hpp"
#include "ui/rendering/i_render_rule.hpp"
#include "ui/rendering/raw_rb_render_rule.hpp"
#include "ui/rendering/raw_rg_render_rule.hpp"
#include "ui/rendering/raw_bw_render_rule.hpp"
#include "ui/plotting/plotter.hpp"

namespace Ui {
class DataView;
}

/**
 * @brief The DataView class.
 */
class DataView : public QWidget
{
    Q_OBJECT

public:
    explicit DataView(int _resolution_x, int _resolution_y, QWidget *parent = 0);
    ~DataView();
    void resizeRenderer(const unsigned int _resolution_x, const unsigned int _resolution_y);
    void clear();

    std::shared_ptr<Renderer> getRenderer();
    std::shared_ptr<Plotter> getAccelPlotter();
    std::shared_ptr<Plotter> getGyroPlotter();

    bool display_events_enabled_;
    bool display_frames_enabled_;

    bool losslessEnabled() const;

    Ui::DataView* getUi();
private:
    //! Update event rate display
    //! @param _meps Mega events per second
    void displayEventRate(float _meps);
    //! Update frame rate display
    //! @param _fps Frames per second
    void displayFrameRate(float _fps);

private slots:
    void on_mixPolarityDisplay_clicked(bool _checked);

    void on_enable_aps_filter_clicked(bool _checked);

    void on_aps_contrast_valueChanged(double _val);

    void on_aps_brightness_valueChanged(int _val);

    void on_eventScale_valueChanged(int _val);

    void on_renderRuleBox_currentIndexChanged(int _index);

    void on_displayEventsEnabled_clicked(bool _checked);

    void on_displayFramesEnabled_clicked(bool _checked);

    void on_plot_gyro_enabled_clicked(bool _checked);

    void on_plot_acceleration_enabled_clicked(bool _checked);

    void on_time_range_ms_edit_valueChanged(int _val);

    void on_gyro_range_edit_valueChanged(double _val);

    void on_accel_range_edit_valueChanged(double _val);

    void on_lossless_enable_checkbox_clicked(bool _checked);

private:
    Ui::DataView *ui;
    std::shared_ptr<Renderer> renderer_;
    std::shared_ptr<Plotter> accel_plotter_;
    std::shared_ptr<Plotter> gyro_plotter_;

    bool lossless_visualization_;

    std::shared_ptr<IRenderRule> rb_render_rule_;
    std::shared_ptr<IRenderRule> rg_render_rule_;
    std::shared_ptr<IRenderRule> bw_render_rule_;
};

#endif // DATA_VIEW_H
