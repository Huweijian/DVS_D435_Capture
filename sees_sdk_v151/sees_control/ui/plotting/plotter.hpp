/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef PLOTTER_H
#define PLOTTER_H

#include <QWidget>
#include <QGroupBox>
#include <QMutex>
#include <QLabel>
#include <memory>

#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include "iness_common/time/definitions.hpp"

class Plotter : public QWidget
{
    Q_OBJECT
public:
    explicit Plotter(QString _title, QString _name0, QString _name1, QString _name2, float _y_range, int _time_range, QWidget *parent = 0);

    QGroupBox *getNumGroupBox();
    void setTimeRangeMs(int _val);
    void setYRange(float _val);
    void clear();
    float getRateHz() const;

private:
    std::shared_ptr<QwtPlot> plot_;
    QwtPlotCurve *curve0_, *curve1_, *curve2_;
    QTimer *timer_;

    // data vectors
    QVector<double> time_sec_;
    QVector<double> data_0_;
    QVector<double> data_1_;
    QVector<double> data_2_;

    double time_range_sec_;
    float y_range_;

    // mutex
    mutable QMutex mutex_;

    // data group box
    QGroupBox *numGroupBox_;
    QGroupBox *createNumGroupBox(QString _title, QString _name0, QString _name1, QString _name2);
    QLabel *val_label_0_;
    QLabel *val_label_1_;
    QLabel *val_label_2_;

signals:

private slots:
    void updatePlot();

public slots:
    void updateData(iness::time::TimeUs _time_us, double _data0, double _data1, double _data2);

};

#endif // PLOTTER_H
