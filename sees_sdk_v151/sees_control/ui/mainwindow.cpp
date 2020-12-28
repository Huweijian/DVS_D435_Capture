/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "ui/mainwindow.hpp"
#include "ui_mainwindow.h"

#include <functional>
#include <QGridLayout>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QtNetwork>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QCloseEvent>

#include "ui/rendering/raw_rb_render_rule.hpp"
#include "ui/ui_strings.hpp"
#include "about_dialog.hpp"
#include "ui_data_view.h"

#include <boost/filesystem.hpp>
#include <chrono>
#include <iomanip>

const unsigned int MainWindow::desactivate_display_buffer_size_ = 60;

constexpr int TCP_PORT = 9999;

inline double get_utc(void){
    auto tp = std::chrono::high_resolution_clock().now();
    double utc = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count()/1e3;
    return utc;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , state_(SeesControlState::IDLE)
    , recording_failed_is_processing_(false)
{
    filename_ = "";

    //Config UI
    ui->setupUi(this);

    setupToolbar();
    setupViews();

    // Source controls
    device_controls_ = new DeviceControls(this);
    device_controls_->hide();
    QObject::connect(device_controls_,SIGNAL(finished(int)),this,SLOT(onSourceConfigureDialogClosed()));
    QObject::connect(device_controls_,SIGNAL(remoteOpenDevice()),this,SLOT(onRemoteOpenDevice()));
    QObject::connect(device_controls_,SIGNAL(remoteCloseDevice()),this,SLOT(onRemoteCloseDevice()));
    QObject::connect(device_controls_,SIGNAL(remoteStartRecording(QString,std::vector<iness::serialization::AedatType>)),this,SLOT(onRemoteStartRecording(QString,std::vector<iness::serialization::AedatType>)));
    QObject::connect(device_controls_,SIGNAL(remoteStopRecording()),this,SLOT(onRemoteStopRecording()));
    QObject::connect(device_controls_,SIGNAL(remoteResetDeviceTime()),this,SLOT(onRemoteDeviceTimeReset()));

    QObject::connect(video_controls_, SIGNAL(play()), this, SLOT(playerPlay()));
    QObject::connect(video_controls_, SIGNAL(pause()), this, SLOT(playerPause()));
    QObject::connect(video_controls_, SIGNAL(stop()), this, SLOT(playerStop()));
    QObject::connect(video_controls_, SIGNAL(setSpeed(double)), this, SLOT(onSetSpeed(double)));
    QObject::connect(video_controls_, SIGNAL(progressChanged(long)), this, SLOT(onChangeProgress(long)));
    QObject::connect(video_controls_, SIGNAL(setEventsPerFrame(uint)), this, SLOT(onChangedEventsPerFrame(uint)));
    QObject::connect(video_controls_, SIGNAL(stepBack()), this, SLOT(playerStepBackward()));
    QObject::connect(video_controls_, SIGNAL(stepForward()), this, SLOT(playerStepForward()));
    QObject::connect(video_controls_, SIGNAL(repeat(bool)), this, SLOT(playerRepeat(bool)));

    QObject::connect(this, SIGNAL(progressChanged(long)), video_controls_, SLOT(setProgress(long)));
    QObject::connect(this, SIGNAL(recordingFailed()), this, SLOT(onRecordingFailed()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupToolbar()
{
    ui->toolBar->addAction(ui->actionOpen);
    ui->toolBar->addSeparator();

    ui->toolBar->addWidget(new QLabel("Select source:"));
    QComboBox *source_select = new QComboBox();
    source_select->addItem(ui_strings::device_source);
    //source_select->addItem(ui_strings::network_source); //TODO: Enable network once functionality is implemented
    ui->toolBar->addWidget(source_select);
    data_source_type_ = DataSourceType::USB_DEVICE;

    QObject::connect(source_select,SIGNAL(currentIndexChanged(QString)),this,SLOT(change_source(QString)));
    QObject::connect(this,SIGNAL(sourceConnected(bool)),source_select,SLOT(setEnabled(bool)));

    ui->toolBar->addAction(ui->actionConnect);
    ui->toolBar->addAction(ui->actionRecord);
    ui->toolBar->addAction(ui->actionConfigure);
    ui->toolBar->addSeparator();

    status_label_ = new QLabel(ui_strings::no_source_connected,this);
    status_label_->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->toolBar->addWidget(status_label_);

    video_controls_ = new VideoControls(this);
    centralWidget()->layout()->addWidget(video_controls_);
    video_controls_->setEnabled(false);
}

void MainWindow::setupViews()
{
    data_view_.reset(new DataView(500,500));
    ui->horizontalLayout->addWidget(data_view_.get(), 0,0);
}

bool MainWindow::switchToIdle()
{
    switch(state_)
    {
    case SeesControlState::IDLE:
        return false;
    case SeesControlState::USB_DATA_ON:
        input_.deviceClose();
        device_controls_->removeDeviceHandle();

        stopInputDataProcessing();

        clearViews();
        // update label and status
        status_label_->setText(ui_strings::no_source_connected);
        break;
    case SeesControlState::NETWORK_DATA_ON:
        // TODO
        // Turn of Network device
        stopInputDataProcessing();

        clearViews();
        // update label and status
        status_label_->setText(ui_strings::no_source_connected);
        break;
    case SeesControlState::USB_DATA_RECORD:
        return false;
    case SeesControlState::NETWORK_DATA_RECORD:
        return false;
    case SeesControlState::DATA_PLAYBACK:
        input_.playerStop();
        data_view_->getRenderer()->wakeAll();
        input_.playerClose();
        data_view_->getRenderer()->startInternalImageTrigger();
        status_label_->setText(ui_strings::no_source_connected);
        break;
    default:
        return false;
    }

    state_ = SeesControlState::IDLE;
    return true;
}

bool MainWindow::switchToUsbDataOn()
{
    if(state_ == SeesControlState::IDLE)
    {
        if(!input_.deviceOpen())
        {
            return false;
        }

        std::function<void()> shutdown_event = [this]()
        {
            this->ui->actionConnect->toggle();
            auto device_ptr = this->input_.deviceGet().lock();
            if(device_ptr)
                this->status_label_->setText( QString::fromStdString("USB device '"+device_ptr->deviceTypeName()+": "+device_ptr->serialNumber()+"' lost connection."));
        };

        // Setup data view
        unsigned int width, height;
        input_.deviceGetSize(width,height);
        data_view_->clear();
        data_view_->resizeRenderer(width, height);

        // Clear input buffer.
        std::queue<std::shared_ptr<aedat::IEventPacket>> empty_queue;
        std::swap(data_input_buffer_, empty_queue ); data_input_utc.clear();

        // Register data callbacks after the device is opened
        input_.deviceRegisterEventPacketCallback(std::bind(&MainWindow::eventPacketProducerCallback, this, std::placeholders::_1));

        // Start data producer.
        if(!input_.deviceStart())
        {
            input_.deviceClose();
            std::cout << "Failed to start the device." << std::endl;
            return false;
        }

        // Start data consumer.
        startInputDataProcessing();

        // set device handle
        device_controls_->setDeviceHandle(input_.deviceGet());

        // update label and status
        auto device_ptr = input_.deviceGet().lock();
        if( device_ptr!=nullptr )
        {
            status_label_->setText( QString::fromStdString("USB device '"+device_ptr->deviceTypeName()+": "+device_ptr->serialNumber()+"' connected."));
        }
        else // oke, that should not really happen actually.
            status_label_->setText(ui_strings::device_connected);
    }
    else if(state_ == SeesControlState::USB_DATA_RECORD)
    {
        stopRecording();
        ui->actionRecord->setChecked(false);
    }
    else
    {
        return false;
    }

    state_ = SeesControlState::USB_DATA_ON;
    return true;
}

bool MainWindow::switchToNetworkDataOn()
{
    if(state_ == SeesControlState::IDLE)
    {
        data_view_->clear();
        // Clear input buffer.
        std::queue<std::shared_ptr<aedat::IEventPacket>> empty_queue;
        std::swap(data_input_buffer_, empty_queue ); data_input_utc.clear();

        // Start data consumer.
        startInputDataProcessing();

        // TODO
        // try to start newtork connection.
        // if fail return false

        status_label_->setText(ui_strings::device_connected);
    }
    else if(state_ == SeesControlState::NETWORK_DATA_RECORD)
    {
        stopRecording();
        ui->actionRecord->setChecked(false);
    }
    else
    {
        return false;
    }

    state_ = SeesControlState::NETWORK_DATA_ON;
    return true;
}

bool MainWindow::switchToUsbDataRecord(QString _file_name, const std::vector<iness::serialization::AedatType>& _packet_types)
{
    iness::aedat::AedatSourceInfo source_info;
    if(state_ == SeesControlState::USB_DATA_ON && input_.deviceGetInfo(source_info))
    {

        ui->actionRecord->setChecked(true);
        // TODO ADD FILTERING TOPICS DIALOG HERE?
        startRecording(source_info, _packet_types, _file_name);
        state_ = SeesControlState::USB_DATA_RECORD;
        return true;
    }
    else
    {
        return false;
    }
}

bool MainWindow::switchToNetworkDataRecord(QString _file_name, const std::vector<iness::serialization::AedatType>& _packet_types)
{
    iness::aedat:: AedatSourceInfo source_info;
    // TODO Check network source info.
    if(state_ == SeesControlState::NETWORK_DATA_ON)
    {
        ui->actionRecord->setChecked(true);
        startRecording(source_info, _packet_types, _file_name);
        state_ = SeesControlState::NETWORK_DATA_RECORD;
        return true;
    }
    else
    {
        return false;
    }
}

bool MainWindow::switchToDataPlayback()
{
    if(state_ == SeesControlState::IDLE || state_ == SeesControlState::DATA_PLAYBACK)
    {
        data_view_->clear();
        data_view_->getRenderer()->stopInternalImageTrigger();
        state_ = SeesControlState::DATA_PLAYBACK;
        return true;
    }
    else
    {
        return false;
    }
}

bool MainWindow::switchToExit()
{
    switch(state_)
    {
    case SeesControlState::IDLE:
        break;
    case SeesControlState::USB_DATA_ON:
        switchToIdle();
        break;
    case SeesControlState::NETWORK_DATA_ON:
        switchToIdle();
        break;
    case SeesControlState::USB_DATA_RECORD:
        switchToUsbDataOn();
        switchToIdle();
        break;
    case SeesControlState::NETWORK_DATA_RECORD:
        switchToNetworkDataOn();
        switchToIdle();
        break;
    case SeesControlState::DATA_PLAYBACK:
        switchToIdle();
        break;
    default:
        break;
    }

    device_controls_->close();

    return true;
}

void MainWindow::startRecording(const iness::aedat::AedatSourceInfo& _source_info,
                                const std::vector<iness::serialization::AedatType>& _types_to_record,
                                const QString _filename)
{
    utc_out.open(std::string("./") + QDateTime::currentDateTime().toString("'utc_'yyyy_MM_dd-hh_mm_ss'.txt'").toUtf8().constData());
    utc_out << std::setprecision(13);
    if (_filename.isEmpty())
    {
        std::string _default_filename = std::string(".") + QDateTime::currentDateTime().toString("'temp_sees_control_recording_'yyyy_MM_dd-hh_mm_ss'.aedat'").toUtf8().constData();
        output_.start(_source_info, _default_filename, _types_to_record);
        is_recording_file_defined_ = false;
    }
    else
    {
        output_.start(_source_info, _filename.toStdString(), _types_to_record);
        is_recording_file_defined_ = true;
    }
}

void MainWindow::stopRecording()
{
    // stop the recording
    std::string current_filename = output_.stop();

    // check if everything was written properly.
    if(!output_.isOk())
    {
        showErrorMessage("Recording failed. Did you run out of disk space? The recorded file might be corrupted.");
    }

    // change name of the file if it was not specified at the beginning
    if (!is_recording_file_defined_)
    {
        std::string _default_filename = QDateTime::currentDateTime().toString("'sees_control_recording_'yyyy_MM_dd-hh_mm_ss'.aedat'").toUtf8().constData();
        QString dir = QDir::currentPath().append(QString::fromStdString("/")).append(QString::fromStdString(_default_filename));
        QString save_filename = QFileDialog::getSaveFileName(this, tr("Save Document"), dir, tr("AEDAT Files (*.aedat)"));
        if(!save_filename.isNull())
        {
            std::string new_filename = save_filename.toUtf8().constData();
            if( std::rename(current_filename.c_str(), new_filename.c_str())!=0 ) // if rename doesn't work (e.g. cause it needs to move the file across different mount points)
            {
                boost::filesystem::copy(current_filename,new_filename);
                boost::filesystem::remove(current_filename);
            }
        }
        else
        {
            std::remove(current_filename.c_str());
        }
    }
}

void MainWindow::connect()
{
    if (data_source_type_ == DataSourceType::USB_DEVICE)
    {
        if(state_ != SeesControlState::IDLE)
        {
            switchToIdle();
        }
        switchToUsbDataOn();
    }
    else if (data_source_type_ == DataSourceType::NETWORK_DEVICE)
    {
        if(state_ != SeesControlState::IDLE)
        {
            switchToIdle();
        }
        switchToNetworkDataOn();
    }

    //    try
    //    {
    //        input_.openTcpStream(_address.toStdString(), TCP_PORT);
    //    }
    //    catch(std::exception& ex)
    //    {
    //        input_.closeTcpStream(); //Close stream. Seems to be necessary, as the underlying socket stays open even on connection failure.
    //        showErrorMessage(ex.what());
    //    }

}

void MainWindow::disconnect()
{
    if (data_source_type_ == DataSourceType::USB_DEVICE)
    {
        switchToIdle();
    }
    else if (data_source_type_ == DataSourceType::NETWORK_DEVICE)
    {
        switchToIdle();
    }

    //    try
    //    {
    //        input_.closeTcpStream();
    //    }
    //    catch(std::exception& ex)
    //    {
    //        showErrorMessage(ex.what());
    //    }
}

void MainWindow::showErrorMessage(const QString &_message)
{
    QMessageBox box;
    box.setText(_message);
    box.setStandardButtons(QMessageBox::Ok);
    box.setDefaultButton(QMessageBox::Ok);
    box.setIcon(QMessageBox::Icon::Critical);
    box.exec();
}

void MainWindow::openFile()
{
    filename_ = QFileDialog::getOpenFileName(this,
                                             tr("Open AEDAT File"),
                                             QSettings().value("last_recording_directory", "").toString(),
                                             tr("AEDAT Files (*.aedat)"));
    if(filename_ == "")
        return;

    playerStop();
    switchToIdle();
    updateGUI();
    QDir current_dir;
    QSettings().setValue("last_recording_directory", current_dir.absoluteFilePath(filename_));

    iness::aedat::AedatSourceInfo source_info;

    try{
        switchToDataPlayback();

        std::shared_ptr<iness::aedat::GenericEventAdvertiser> advertiser =
                std::make_shared<iness::aedat::GenericEventAdvertiser>(std::bind(&MainWindow::handleEventPacketVisualization, this, std::placeholders::_1),
                                                                       std::bind(&MainWindow::handleEventVisualization, this, std::placeholders::_1, std::placeholders::_2));

        input_.playerOpen(filename_.toStdString(), advertiser, source_info);
        status_label_->setText( QString::fromStdString("Playing back data from "+source_info.source_description+" device.") );
        input_.playerRegisterProgressCallback(std::bind(&MainWindow::updateProgress, this, std::placeholders::_1));
        input_.playerRegisterFrameTrigger(std::bind(&Renderer::emitExternalTrigger, data_view_->getRenderer()),30);


        auto file_info = input_.playerGetFileInfo();
        video_controls_->setInterval(file_info.start_time, file_info.end_time);

        if(source_info.source_info!=nullptr)
        {
            // Setup data view
            if((*source_info.source_info).hasEntry("DvsWidth") && (*source_info.source_info).hasEntry("DvsHeight"))
            {
                int width, height;
                iness::ptree::read( source_info.source_info->entry("DvsWidth"),width);
                iness::ptree::read( source_info.source_info->entry("DvsHeight"),height);
            	data_view_->resizeRenderer(width,height);
            }
        }

    }
    catch(std::exception& ex)
    {
        QString msg = "Could not open file: ";
        msg += ex.what();
        showErrorMessage(msg);
    }
    catch(const char* error)
    {
        QString msg = "Could not open file: ";
        msg += QString(error);
        showErrorMessage(msg);
    }
    catch(...)
    {
        QString msg = "Could not open file.";
        showErrorMessage(msg);
    }
    clearViews();
}

void MainWindow::clearViews()
{
    data_view_->clear();
}

void MainWindow::startInputDataProcessing()
{
    run_input_data_processing_thread_ = true;
    input_data_processing_thread_ = std::thread(&MainWindow::inputDataProcessingLoop, this);
}

void MainWindow::stopInputDataProcessing()
{
    run_input_data_processing_thread_ = false;
    data_input_buffer_cv_.notify_all();
    if(input_data_processing_thread_.joinable())
    {
        input_data_processing_thread_.join();
    }
    else
    {
        // TODO remove
        throw std::runtime_error("stopInputDataProcessing() was called without starting the thread");
    }
}

bool MainWindow::inputDataAvailableBlocking()
{
    std::unique_lock<std::mutex> lock(data_input_buffer_mutex_);

    if(data_input_buffer_.size() == 0)
    {
        auto cv_status = data_input_buffer_cv_.wait_for( lock, std::chrono::duration<int,std::milli>(100) );
        if( cv_status==std::cv_status::timeout )
            return false;

        if(data_input_buffer_.size() > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}

void MainWindow::inputDataProcessingLoop()
{
    while(run_input_data_processing_thread_)
    {
        if(inputDataAvailableBlocking())
        {
            unsigned int input_buffer_size = 0;
            data_input_buffer_mutex_.lock();
            std::shared_ptr<aedat::IEventPacket> current_packet = data_input_buffer_.front();
            double utc = data_input_utc.front();
            data_input_buffer_.pop(); data_input_utc.pop_front();
            input_buffer_size = data_input_buffer_.size();
            data_input_buffer_mutex_.unlock();

            // If too much data is buffered in data_input_buffer_, pause visualization to speed up processing.
            if( data_view_->losslessEnabled() || input_buffer_size < desactivate_display_buffer_size_)
            {
                handleEventPacketVisualization(current_packet);
            }

            handleEventPacketRecording(current_packet, utc);
        }
    }
}

void MainWindow::handleEventPacketVisualization(std::shared_ptr<aedat::IEventPacket> _event_packet)
{
    if(_event_packet->is(iness::serialization::AedatType::POLARITY_EVENT))
    {
        if (data_view_.get()->display_events_enabled_)
        {
            iness::PolarityEventPacket& polarity_packet = *(_event_packet->interpretAs<iness::PolarityEventPacket>());
            for(iness::PolarityEvent& polarity_event : polarity_packet)
            {
                visualizeEvent(polarity_event, polarity_packet);
            }
        }
    }
    else if(_event_packet->is(iness::serialization::AedatType::IMU6_EVENT))
    {
        iness::Imu6EventPacket& imu_packet = *(_event_packet->interpretAs<iness::Imu6EventPacket>());

        for(const iness::Imu6Event& imu6_event : imu_packet)
        {
            iness::time::TimeUs time_us = imu6_event.getTimestampUs(imu_packet.header().event_ts_overflow);
            data_view_->getAccelPlotter()->updateData(time_us,imu6_event.getAccelerationX(),imu6_event.getAccelerationY(),imu6_event.getAccelerationZ());
            data_view_->getGyroPlotter()->updateData(time_us,imu6_event.getGyroX(),imu6_event.getGyroY(),imu6_event.getGyroZ());
            data_view_->getUi()->label_imu_rate->setText( QString::number(data_view_->getAccelPlotter()->getRateHz(),'f',2) );
        }
    }
    else if(_event_packet->is(iness::serialization::AedatType::FRAME_EVENT))
    {
        iness::FrameEventPacket& frame_packet = *(_event_packet->interpretAs<iness::FrameEventPacket>());
        if (data_view_.get()->display_frames_enabled_)
        {
            for( auto& frame_event : frame_packet )
            {
                cv::Mat img = frame_event.getImage(); // is 16bit
                if(!img.empty())
                {
                    data_view_->getRenderer()->updateImage(img, frame_event.getTsEndFrame());
                }
            }
        }
    }
}


void MainWindow::visualizeEvent( iness::PolarityEvent& _event, iness::PolarityEventPacket& _packet)
{
    data_view_->getRenderer()->updatePolarity(_event, _packet);
}

void MainWindow::handleEventVisualization(std::shared_ptr<aedat::IEventPacket> _event_packet, unsigned int event_index)
{
    if(_event_packet->is(iness::serialization::AedatType::POLARITY_EVENT))
    {
        iness::PolarityEventPacket& polarity_packet = *(_event_packet->interpretAs<iness::PolarityEventPacket>());

        if (data_view_.get()->display_events_enabled_)
        {
            data_view_->getRenderer()->updatePolarity(polarity_packet[event_index], polarity_packet);
        }
    }
    else if(_event_packet->is(iness::serialization::AedatType::IMU6_EVENT))
    {
        iness::Imu6EventPacket& imu_packet = *(_event_packet->interpretAs<iness::Imu6EventPacket>());

        const auto imu6_event = imu_packet[event_index];
        double timeMs = imu6_event.getTimestampUs(0)/1000.0;
        data_view_->getAccelPlotter()->updateData(timeMs,imu6_event.getAccelerationX(),imu6_event.getAccelerationY(),imu6_event.getAccelerationZ());
        data_view_->getGyroPlotter()->updateData(timeMs,imu6_event.getGyroX(),imu6_event.getGyroY(),imu6_event.getGyroZ());

    }
    else if(_event_packet->is(iness::serialization::AedatType::FRAME_EVENT))
    {
        iness::FrameEventPacket& frame_packet = *(_event_packet->interpretAs<iness::FrameEventPacket>());
        if (data_view_.get()->display_frames_enabled_)
        {
            cv::Mat img = frame_packet[event_index].getImage(); // is 16bit
            if(!img.empty())
            {
                data_view_->getRenderer()->updateImage(img, frame_packet[event_index].getTsEndFrame());
            }
        }
    }
}

void MainWindow::handleEventPacketRecording(std::shared_ptr<aedat::IEventPacket> _event_packet, double utc)
{
    if(output_.isRecording())
    {
        // 保存时间戳
        if (_event_packet->is(iness::serialization::AedatType::FRAME_EVENT)) {
            iness::FrameEventPacket& frame_packet = *(_event_packet->interpretAs<iness::FrameEventPacket>());
            for (auto& frame_event : frame_packet) {
                double ts = frame_event.getTimestampUs(frame_packet.header().event_ts_overflow) / 1e6;
                utc_out << ts << " " << utc << std::endl;
            }
        }

        output_.writePacket(_event_packet);
        if(!output_.isOk())
        {
            emit recordingFailed();
        }
    }
}



void MainWindow::eventPacketProducerCallback(std::shared_ptr<iness::aedat::IEventPacket> _event_packet)
{
    std::lock_guard<std::mutex> lock(data_input_buffer_mutex_);
    data_input_buffer_.push(_event_packet);
    data_input_utc.push_back(get_utc());
    data_input_buffer_cv_.notify_all();
}

void MainWindow::updateProgress(int64_t _time)
{
    emit progressChanged(_time);
}

void MainWindow::updateGUI() {
    switch(state_)
    {
    case SeesControlState::IDLE:
        // enable/disable open action
        ui->actionOpen->setEnabled(true);
        // enable/disable source selection
        emit sourceConnected(true);
        // enable/disable configure action
        ui->actionConfigure->setEnabled(true);
        // enable/disable connect action
        ui->actionConnect->setEnabled(true);
        ui->actionConnect->setChecked(false);
        // enable/disable record action
        ui->actionRecord->setEnabled(false);
        // disable video controls
        video_controls_->setEnabled(false);
        video_controls_->uiStop();
        break;
    case SeesControlState::USB_DATA_ON:
        // enable/disable open action
        ui->actionOpen->setEnabled(true);
        // enable/disable source selection
        emit sourceConnected(false);
        // enable/disable configure action
        ui->actionConfigure->setEnabled(true);
        // enable/disable connect action
        ui->actionConnect->setEnabled(true);
        ui->actionConnect->setChecked(true);
        // enable/disable record action
        ui->actionRecord->setEnabled(true);
        // disable video controls
        video_controls_->setEnabled(false);
        break;
    case SeesControlState::NETWORK_DATA_ON:
        // enable/disable open action
        ui->actionOpen->setEnabled(true);
        // enable/disable source selection
        emit sourceConnected(false);
        // enable/disable configure action
        ui->actionConfigure->setEnabled(true);
        // enable/disable connect action
        ui->actionConnect->setEnabled(true);
        ui->actionConnect->setChecked(true);
        // enable/disable record action
        ui->actionRecord->setEnabled(true);
        // disable video controls
        video_controls_->setEnabled(false);
        break;
    case SeesControlState::USB_DATA_RECORD:
        // enable/disable open action
        ui->actionOpen->setEnabled(false);
        // enable/disable source selection
        emit sourceConnected(false);
        // enable/disable configure action
        ui->actionConfigure->setEnabled(true);
        // enable/disable connect action
        ui->actionConnect->setEnabled(false);
        // enable/disable record action
        ui->actionRecord->setEnabled(true);
        // disable video controls
        video_controls_->setEnabled(false);
        break;
    case SeesControlState::NETWORK_DATA_RECORD:
        // enable/disable open action
        ui->actionOpen->setEnabled(false);
        // enable/disable source selection
        emit sourceConnected(false);
        // enable/disable configure action
        ui->actionConfigure->setEnabled(true);
        // enable/disable connect action
        ui->actionConnect->setEnabled(false);
        // enable/disable record action
        ui->actionRecord->setEnabled(true);
        // disable video controls
        video_controls_->setEnabled(false);
        break;
    case SeesControlState::DATA_PLAYBACK:
        // enable/disable open action
        ui->actionOpen->setEnabled(true);
        // enable/disable source selection
        emit sourceConnected(true);
        // enable/disable configure action
        ui->actionConfigure->setEnabled(true);
        // enable/disable connect action
        ui->actionConnect->setEnabled(true);
        ui->actionConnect->setChecked(false);
        // enable/disable record action
        ui->actionRecord->setEnabled(false);
        // disable video controls
        video_controls_->setEnabled(true);
        break;
    default:
        break;
    }
}

void MainWindow::change_source(QString _source)
{
    if (_source == ui_strings::device_source)
    {
        data_source_type_ = DataSourceType::USB_DEVICE;
    }
    else if (_source == ui_strings::network_source)
    {
        data_source_type_ = DataSourceType::NETWORK_DEVICE;
    }
}

void MainWindow::onDisconnect(QString _message)
{
    showErrorMessage(_message);
}

void MainWindow::on_actionClose_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog* aboutDialog = new AboutDialog(this);
    aboutDialog->show();
}

void MainWindow::on_actionConnect_triggered()
{
    if(ui->actionConnect->isChecked())
    {
        connect();
    }
    else
    {
        disconnect();
    }

    updateGUI();
}

void MainWindow::on_actionRecord_triggered()
{
    if(ui->actionRecord->isChecked())
    {
        if(state_ == SeesControlState::USB_DATA_ON)
        {
            switchToUsbDataRecord();
        }
        else if(state_ == SeesControlState::NETWORK_DATA_ON)
        {
            switchToNetworkDataRecord();
        }
    }
    else
    {
        if(state_ == SeesControlState::USB_DATA_RECORD)
        {
            switchToUsbDataOn();
        }
        else if(state_ == SeesControlState::NETWORK_DATA_RECORD)
        {
            switchToNetworkDataOn();
        }
    }

    updateGUI();
}


void MainWindow::on_actionOpen_triggered()
{
    if( state_ == SeesControlState::NETWORK_DATA_ON )
    {
        switchToIdle();
    }

    openFile();
    updateGUI();
}

void MainWindow::playerPlay()
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerPlay();
    }
    updateGUI();
}

void MainWindow::playerPause()
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerPause();
    }
    updateGUI();
}

void MainWindow::playerStop()
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerStop();
    }
    updateGUI();
}

void MainWindow::playerStepForward()
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerStep(true);
    }
}

void MainWindow::playerStepBackward()
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerStep(false);
    }
}

void MainWindow::playerRepeat(bool _checked)
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerLoop(_checked);
    }
}

void MainWindow::onSetSpeed(double _speed)
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerSetSpeed(_speed);
    }
}

void MainWindow::onChangeProgress(long _timestamp)
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerSetPlaybackTime(_timestamp);
    }
}

void MainWindow::onChangedEventsPerFrame(unsigned int _events_per_frame)
{
    if(state_ == SeesControlState::DATA_PLAYBACK)
    {
        input_.playerSetEventsPerFrame(_events_per_frame);
    }
}

void MainWindow::on_actionConfigure_triggered(bool _checked)
{
    if (data_source_type_ == DataSourceType::USB_DEVICE)
    {
        if (_checked)
        {
            device_controls_->show();
        }
        else
        {
            device_controls_->hide();
        }
    }
    else if (data_source_type_ == DataSourceType::NETWORK_DEVICE)
    {

    }
}

void MainWindow::onRecordingFailed()
{
    if(recording_failed_is_processing_)
    {
        return;
    }

    recording_failed_is_processing_ = true;

    if(state_ == SeesControlState::USB_DATA_RECORD)
    {
        switchToUsbDataOn();
    }
    else if(state_ == SeesControlState::NETWORK_DATA_RECORD)
    {
        switchToNetworkDataOn();
    }

    updateGUI();
    recording_failed_is_processing_ = false;
}

void MainWindow::onSourceConfigureDialogClosed()
{
    ui->actionConfigure->setChecked(false);
}

void MainWindow::closeEvent(QCloseEvent *_event)
{
    switchToExit();
    // accept close event
    _event->accept();
}

void MainWindow::onRemoteOpenDevice() {
    connect();
}

void MainWindow::onRemoteCloseDevice() {
    disconnect();
}

void MainWindow::onRemoteStartRecording(QString _filename, std::vector<iness::serialization::AedatType> _packet_types) {
    if(state_ == SeesControlState::USB_DATA_ON)
    {
        switchToUsbDataRecord(_filename, _packet_types);
    }
    else if(state_ == SeesControlState::NETWORK_DATA_ON)
    {
        switchToNetworkDataRecord(_filename, _packet_types);
    }
    updateGUI();
}

void MainWindow::onRemoteStopRecording() {
    if(state_ == SeesControlState::USB_DATA_RECORD)
    {
        switchToUsbDataOn();
    }
    else if(state_ == SeesControlState::NETWORK_DATA_RECORD)
    {
        switchToNetworkDataOn();
    }
    updateGUI();
}

void MainWindow::onRemoteDeviceTimeReset()
{
    data_view_->clear();
}
