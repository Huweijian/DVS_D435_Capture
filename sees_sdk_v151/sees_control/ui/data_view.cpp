/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "data_view.hpp"

#include <QGroupBox>
#include <QResizeEvent>

#include <functional>
#include <iostream>

#include "ui_data_view.h"


DataView::DataView(int _resolution_x, int _resolution_y, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataView),
    lossless_visualization_(false)
{
    ui->setupUi(this);

    // Setup SEES data renderer
    rb_render_rule_.reset(new RawRBRenderRule());
    rg_render_rule_.reset(new RawRGRenderRule());
    bw_render_rule_.reset(new RawBWRenderRule());
    renderer_.reset(new Renderer(_resolution_x, _resolution_y, rb_render_rule_));
    renderer_->setEventRateCallback(std::bind(&DataView::displayEventRate,this,std::placeholders::_1));
    renderer_->setFrameRateCallback(std::bind(&DataView::displayFrameRate,this,std::placeholders::_1));
    renderer_->startInternalImageTrigger();

    QHBoxLayout* render_layout = new QHBoxLayout(ui->rawDataBox);
    render_layout->addWidget(renderer_.get(),Qt::AlignHCenter);

    display_events_enabled_ = true;
    display_frames_enabled_ = true;

    // Setup IMU data renderer
    accel_plotter_.reset(new Plotter("Accelerometer","X","Y","Z",ui->accel_range_edit->value(),ui->time_range_ms_edit->value(),this));
    gyro_plotter_.reset(new Plotter("Gyroscope","X","Y","Z",ui->gyro_range_edit->value(),ui->time_range_ms_edit->value(),this));

    QVBoxLayout* plotter_layout = new QVBoxLayout(ui->imuRawDataBox);
    plotter_layout->addWidget(accel_plotter_.get(),Qt::AlignHCenter);
    plotter_layout->addWidget(gyro_plotter_.get(),Qt::AlignHCenter);

    QHBoxLayout* imu_label_layout = new QHBoxLayout();
    imu_label_layout->addWidget(accel_plotter_.get()->getNumGroupBox());
    imu_label_layout->addWidget(gyro_plotter_.get()->getNumGroupBox());

    ui->imuVGroupLayout->addLayout(imu_label_layout);
    ui->imuVGroupLayout->addSpacerItem(new QSpacerItem(20,40,QSizePolicy::Minimum,QSizePolicy::Expanding));

    ui->eventScale->setEnabled(ui->mixPolarityDisplay->checkState());
    ui->eventScaleLabel->setEnabled(ui->mixPolarityDisplay->checkState());
    on_enable_aps_filter_clicked(false);
}

DataView::~DataView()
{
    delete ui;
}

void DataView::resizeRenderer(const unsigned int _resolution_x, const unsigned int _resolution_y)
{
    renderer_->setInputSize(_resolution_y,_resolution_x);
}

void DataView::clear()
{
    if(accel_plotter_ != nullptr && gyro_plotter_ != nullptr)
    {
        accel_plotter_->clear();
        gyro_plotter_->clear();
    }

    if(renderer_!= nullptr)
    {
        renderer_->clear();
    }
}

bool DataView::losslessEnabled() const
{
    return lossless_visualization_;
}

std::shared_ptr<Plotter> DataView::getAccelPlotter()
{
    return accel_plotter_;
}

std::shared_ptr<Plotter> DataView::getGyroPlotter()
{
    return gyro_plotter_;
}

Ui::DataView* DataView::getUi()
{
    return ui;
}

std::shared_ptr<Renderer> DataView::getRenderer()
{
    return renderer_;
}

void DataView::displayEventRate(float _meps)
{
    QString event_rate_text = QString::number(_meps,'f',2).append(" Meps");
    ui->labelMeps->setText(event_rate_text);
}

void DataView::displayFrameRate(float _fps)
{
    QString frame_rate_text = QString::number(_fps,'f',1).append(" Fps");
    ui->labelFps->setText(frame_rate_text);
}

void DataView::on_mixPolarityDisplay_clicked(bool _checked)
{
    if(_checked)
    {
        if(ui!=nullptr)
        {
            ui->eventScale->setEnabled(true);
            ui->eventScaleLabel->setEnabled(true);
        }
        renderer_->setPolarityMixRenderModus(Renderer::PolarityMixRenderModus::MIX);
    }
    else
    {
        {
            ui->eventScale->setEnabled(false);
            ui->eventScaleLabel->setEnabled(false);
        }
        renderer_->setPolarityMixRenderModus(Renderer::PolarityMixRenderModus::OVERWRITE);
    }
}

void DataView::on_enable_aps_filter_clicked(bool _checked)
{
    renderer_->setEnableApsFilter(_checked);

    ui->enable_aps_filter->setChecked(_checked);
    ui->aps_brightness->setEnabled(_checked);
    ui->aps_contrast->setEnabled(_checked);
    ui->aps_brightness_text->setEnabled(_checked);
    ui->aps_contrast_text->setEnabled(_checked);
}

void DataView::on_aps_contrast_valueChanged(double _val)
{
    renderer_->setApsContrast(_val);
}

void DataView::on_aps_brightness_valueChanged(int _val)
{
    renderer_->setApsBrightness(_val);
}

void DataView::on_eventScale_valueChanged(int _val)
{
   renderer_->setMaxEventCount(_val);
}

void DataView::on_renderRuleBox_currentIndexChanged(int _index)
{
    ui->mixPolarityDisplay->setEnabled(true);
    if (_index == 0)
    {
        renderer_->setRenderRule(rb_render_rule_);
    }
    else if (_index == 1)
    {
        renderer_->setRenderRule(rg_render_rule_);
    }
    else if (_index == 2)
    {
        renderer_->setRenderRule(bw_render_rule_);
        ui->mixPolarityDisplay->setChecked(false);
        on_mixPolarityDisplay_clicked(false);
        ui->mixPolarityDisplay->setEnabled(false);
    }
    else
    {
        renderer_->setRenderRule(rb_render_rule_);
    }
    renderer_->clear();
}

void DataView::on_displayEventsEnabled_clicked(bool _checked)
{
    display_events_enabled_ = _checked;
    renderer_->clear();
}

void DataView::on_displayFramesEnabled_clicked(bool _checked)
{
    display_frames_enabled_ = _checked;
    renderer_->clear();
}

void DataView::on_plot_gyro_enabled_clicked(bool _checked)
{
    if (_checked)
        gyro_plotter_.get()->show();
    else
        gyro_plotter_.get()->hide();
}

void DataView::on_plot_acceleration_enabled_clicked(bool _checked)
{
    if (_checked)
        accel_plotter_.get()->show();
    else
        accel_plotter_.get()->hide();
}

void DataView::on_time_range_ms_edit_valueChanged(int _val)
{
    accel_plotter_.get()->setTimeRangeMs(_val);
    gyro_plotter_.get()->setTimeRangeMs(_val);
}

void DataView::on_gyro_range_edit_valueChanged(double _val)
{
    gyro_plotter_.get()->setYRange(_val);
}

void DataView::on_accel_range_edit_valueChanged(double _val)
{
    accel_plotter_.get()->setYRange(_val);
}

void DataView::on_lossless_enable_checkbox_clicked(bool _checked)
{
    lossless_visualization_ = _checked;
}
