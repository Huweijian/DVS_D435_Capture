/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef VIDEO_CONTROLS_H
#define VIDEO_CONTROLS_H

#include <QWidget>

namespace Ui {
class VideoControls;
}

class VideoControls : public QWidget
{
    Q_OBJECT

public:
    explicit VideoControls(QWidget *parent = 0);
    ~VideoControls();
    void setInterval(long _start_time, long _end_time);
    void uiStop();

signals:
    void play();
    void pause();
    void stop();
    void setSpeed(double);
    void setEventsPerFrame(unsigned int);
    void progressChanged(long);
    void stepBack();
    void stepForward();
    void repeat(bool);

public slots:
    void setProgress(long _timestamp);

private slots:
    void on_pushButtonStop_clicked();

    void on_doubleSpinBoxSpeed_valueChanged(double arg1);

    void on_lineEditTime_editingFinished();

    void on_horizontalSliderProgress_sliderPressed();

    void on_horizontalSliderProgress_sliderReleased();

    void on_spinBoxEventsPerFrame_valueChanged(int arg1);

    void on_radioButtonRate_toggled(bool checked);

    void on_pushButtonPlay_clicked(bool checked);

    void on_pushButtonBackward_clicked();

    void on_pushButtonForward_clicked();

    void on_pushButtonRepeat_clicked(bool _checked);

private:
    void play(bool _play);
    void uiPlay(bool _play);

    Ui::VideoControls *ui;
    long start_time_;
    long end_time_;
};

#endif // VIDEO_CONTROLS_H
