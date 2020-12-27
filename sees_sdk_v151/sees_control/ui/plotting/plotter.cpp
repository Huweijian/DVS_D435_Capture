/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include <QGridLayout>
#include <QTimer>
#include <qwt/qwt_legend.h>
#include <qwt/qwt_plot_grid.h>
#include <qwt/qwt_scale_draw.h>
#include <qwt/qwt_scale_widget.h>
#include <QTime>

#include "plotter.hpp"

class DrawAxisTimeFormat: public QwtScaleDraw
{
public:
    DrawAxisTimeFormat() = default;
    virtual QwtText label( double _time_s ) const override
    {
        int total_ms =_time_s*1000;

        int milliseconds = total_ms%1000;
        int seconds = (total_ms/1000)%60;
        int minutes = (total_ms/60000)%60;
        int hours = total_ms/3600000;
        QTime current_time(hours,minutes,seconds,milliseconds);

        QString format; // adapt the time format as needed
        if(hours!=0)
        {
            format = "h:mm:ss";
        }
        else if(minutes==0 && seconds <=2)
        {
            format = "m:ss.zzz";
        }
        else
        {
            format = "m:ss";
        }

        return current_time.toString(format);
    }
};

Plotter::Plotter(QString _title, QString _name0, QString _name1, QString _name2, float _y_range, int _time_range, QWidget *parent)
: QWidget(parent)
, plot_(new QwtPlot(this))
{
    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->addWidget(plot_.get(),0,0,1,1);
    gridLayout->setContentsMargins(0,0,0,0);

    plot_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    // default values
    time_range_sec_ = double(_time_range) / 1000.0;
    y_range_ = _y_range;

    // set up plot
    plot_->setAxisScale(QwtPlot::yLeft,-_y_range,_y_range);


    QLinearGradient plotGradient;
    plotGradient.setStart(0, 0);
    plotGradient.setFinalStop(0, 350);
    plotGradient.setColorAt(0, QColor(80, 80, 80));
    plotGradient.setColorAt(1, QColor(50, 50, 50));
    plot_->setCanvasBackground(plotGradient);

    /* When a new time label appears at the most right of the x-axis, qwt needs
     * additional space to draw it. This is perceived as jumping by the user.
     * We add a little space to minimize the effect.
     */
    QwtScaleWidget* scale_widget = plot_->axisWidget( QwtPlot::xBottom );
    const int fmh = QFontMetrics( scale_widget->font() ).height();
    scale_widget->setMinBorderDist( 0, fmh );


    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajorPen(QPen(QColor(160, 160, 160), 1, Qt::DotLine));
    grid->setMinorPen(QPen(QColor(100, 100, 100), 1, Qt::DotLine));
    grid->attach(plot_.get());

    plot_->setAxisScaleDraw( QwtPlot::xBottom, new DrawAxisTimeFormat() );

    QLinearGradient axisRectGradient;
    axisRectGradient.setStart(0, 0);
    axisRectGradient.setFinalStop(0, 350);
    axisRectGradient.setColorAt(0, QColor(80, 80, 80));
    axisRectGradient.setColorAt(1, QColor(30, 30, 30));

    curve0_ = new QwtPlotCurve(_name0);
    curve0_->setPen(Qt::red);
    curve0_->attach(plot_.get());

    curve1_ = new QwtPlotCurve(_name1);
    curve1_->setPen(Qt::green);
    curve1_->attach(plot_.get());

    curve2_ = new QwtPlotCurve(_name2);
    curve2_->setPen(Qt::blue);
    curve2_->attach(plot_.get());

    // create widget for dispalying numerical values
    numGroupBox_ = createNumGroupBox(_title,_name0,_name1,_name2);

    // setup a timer that repeatedly calls realtimeDataSlot
    timer_ =  new QTimer();
    connect(timer_, SIGNAL(timeout()), this, SLOT(updatePlot()));
    timer_->start(20); // Interval 0 means to refresh as fast as possible

}

QGroupBox *Plotter::createNumGroupBox(QString _title, QString _name0, QString _name1, QString _name2)
{
    QGroupBox *dataBox = new QGroupBox(_title);
    QVBoxLayout *layout = new QVBoxLayout(dataBox);

    dataBox->setLayout(layout);

    QHBoxLayout *label0Layout = new QHBoxLayout();
    label0Layout->addWidget(new QLabel(_name0+":"));
    label0Layout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    val_label_0_ = new QLabel(dataBox);
    label0Layout->addWidget(val_label_0_);

    QHBoxLayout *label1Layout = new QHBoxLayout();
    label1Layout->addWidget(new QLabel(_name1+":"));
    label1Layout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    val_label_1_ = new QLabel(dataBox);
    label1Layout->addWidget(val_label_1_);

    QHBoxLayout *label2Layout = new QHBoxLayout();
    label2Layout->addWidget(new QLabel(_name2+":"));
    label2Layout->addSpacerItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    val_label_2_ = new QLabel(dataBox);
    label2Layout->addWidget(val_label_2_);

    layout->addLayout(label0Layout);
    layout->addLayout(label1Layout);
    layout->addLayout(label2Layout);

    return dataBox;
}

QGroupBox *Plotter::getNumGroupBox()
{
    return numGroupBox_;
}

void Plotter::updateData(iness::time::TimeUs _time_us, double _data0, double _data1, double _data2)
{

    QMutexLocker locker(&mutex_);
    if( !time_sec_.empty() && time_sec_.back() > _time_us / 1000.0 / 1000.0 )
    {
        locker.unlock();
        clear();
        return;
    }

    time_sec_.append(_time_us / 1000.0 / 1000.0);

    data_0_.append(_data0);
    data_1_.append(_data1);
    data_2_.append(_data2);

    while( time_sec_.first() < (time_sec_.last() - time_range_sec_) )
    {
        time_sec_.pop_front();
        data_0_.pop_front();
        data_1_.pop_front();
        data_2_.pop_front();
    }
}

void Plotter::updatePlot() {
    QMutexLocker locker(&mutex_);
    if (!time_sec_.isEmpty())
    {
        // add data vectors to graphs

        curve0_->setSamples(time_sec_, data_0_);
        curve1_->setSamples(time_sec_, data_1_);
        curve2_->setSamples(time_sec_, data_2_);

        plot_->setAxisScale(QwtPlot::yLeft, -y_range_, y_range_);
        plot_->setAxisScale(QwtPlot::xBottom, time_sec_.first(), time_sec_.last());
        plot_->replot();

        // update the labels
        val_label_0_->setText(QString::number(data_0_.last(),'g',4));
        val_label_1_->setText(QString::number(data_1_.last(),'g',4));
        val_label_2_->setText(QString::number(data_2_.last(),'g',4));
    }
}

void Plotter::setTimeRangeMs(int _val)
{
    QMutexLocker locker(&mutex_);
    time_range_sec_ = double(_val) / 1000.0;
}

void Plotter::setYRange(float _val)
{
    QMutexLocker locker(&mutex_);
    y_range_ = _val;
}

void Plotter::clear()
{
    QMutexLocker locker(&mutex_);
    time_sec_.clear();
    data_0_.clear();
    data_1_.clear();
    data_2_.clear();
}

float Plotter::getRateHz() const
{
    QMutexLocker locker(&mutex_);

    if( time_sec_.size() < 2 )
        return 0.0f;

    return time_sec_.size()/(time_sec_.back()-time_sec_.front());
}
