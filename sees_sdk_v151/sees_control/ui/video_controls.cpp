/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "video_controls.hpp"
#include "ui_video_controls.h"

#include <QDebug>

VideoControls::VideoControls(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoControls)
{
    ui->setupUi(this);
}

VideoControls::~VideoControls()
{
    delete ui;
}

void VideoControls::setInterval(long _start_time, long _end_time)
{
    start_time_ = _start_time;
    end_time_ = _end_time;
    setProgress(start_time_);
}

void VideoControls::setProgress(long _timestamp)
{
    if(end_time_ == start_time_)
        return;
    int progress = 100 * (_timestamp - start_time_)/(end_time_ - start_time_);
    ui->horizontalSliderProgress->setValue(progress);
    QString text = QString::number(_timestamp);
    ui->lineEditTime->setText(text);
}

void VideoControls::on_pushButtonStop_clicked()
{
    uiStop();
    emit stop();
}

void VideoControls::play(bool _play)
{
    ui->pushButtonPlay->setChecked(_play);
    if(_play)
        emit play();
    else
        emit pause();
}

void VideoControls::uiPlay(bool _play)
{
    ui->pushButtonPlay->setChecked(_play);
}

void VideoControls::uiStop()
{
    ui->pushButtonPlay->setChecked(false);
}

void VideoControls::on_doubleSpinBoxSpeed_valueChanged(double arg1)
{
    emit setSpeed(arg1);
}

void VideoControls::on_lineEditTime_editingFinished()
{
    if(ui->lineEditTime->text() == "" || ui->lineEditTime->text() == nullptr)
        return;
    long timestamp = ui->lineEditTime->text().toLong();
    emit progressChanged(timestamp);
}

void VideoControls::on_horizontalSliderProgress_sliderPressed()
{
    emit  pause();
}

void VideoControls::on_horizontalSliderProgress_sliderReleased()
{
    int position = ui->horizontalSliderProgress->value();
    int64_t timestamp = (position+1)/100.0 * (end_time_ - start_time_) + start_time_; // +1 to prevent rounding error
    emit progressChanged(timestamp);
    if(ui->pushButtonPlay->isChecked())
    {
        emit  play();
    }
}

void VideoControls::on_spinBoxEventsPerFrame_valueChanged(int arg1)
{
    if(ui->radioButtonRate->isChecked())
        setEventsPerFrame(arg1);
}

void VideoControls::on_radioButtonRate_toggled(bool checked)
{
    int value = checked ? ui->spinBoxEventsPerFrame->value() : 0 ;
    setEventsPerFrame(value);
}

void VideoControls::on_pushButtonPlay_clicked(bool checked)
{
    play (checked);
}

void VideoControls::on_pushButtonBackward_clicked()
{
    emit stepBack();
}

void VideoControls::on_pushButtonForward_clicked()
{
    emit stepForward();
}

void VideoControls::on_pushButtonRepeat_clicked(bool _checked)
{
    emit repeat(_checked);
}
