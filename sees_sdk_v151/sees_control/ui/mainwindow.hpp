/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "data_view.hpp"

#include "device_controls.hpp"
#include "video_controls.hpp"
#include <iness_common/streaming/aedat_reader/aedat_input.hpp>
#include <iness_common/streaming/aedat_writer/aedat_output.hpp>

using namespace iness;

namespace Ui {
class MainWindow;
}

/**
 * @brief The MainWindow class.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    /* *****************************************************************
     *   The gui states. Use switchTo... to change the gui state.      *
     *                                                                 *
     *                                                                 *
     *           ,---[USB_DATA_ON]-----[USB_DATA_RECORD]               *
     *          /                                                      *
     *         /                                                       *
     * -->[IDLE]------[NETWORK_DATA_ON]---[NETWORK_DATA_RECORD]        *
     *     |   \                                                       *
     *     |    \                                                      *
     *     |     '---[DATA_PLAYBACK]                                   *
     *   [EXIT]                                                        *
     *                                                                 *
     * *****************************************************************
     */

    enum class SeesControlState
    {
        IDLE,
        USB_DATA_ON,
        NETWORK_DATA_ON,
        USB_DATA_RECORD,
        NETWORK_DATA_RECORD,
        DATA_PLAYBACK,
        EXIT
    };

    // State transition functions
    bool switchToIdle();
    bool switchToUsbDataOn();
    bool switchToNetworkDataOn();
    bool switchToUsbDataRecord(QString _file_name = QString(), const std::vector<iness::serialization::AedatType>& _packet_types = std::vector<iness::serialization::AedatType>());
    bool switchToNetworkDataRecord(QString _file_name = QString(), const std::vector<iness::serialization::AedatType>& _packet_types = std::vector<iness::serialization::AedatType>());
    bool switchToDataPlayback();
    bool switchToExit();

    /**
     * @brief Initializes toolbar controls which cannot be added via the designer.
     */
    void setupToolbar();

    /**
     * @brief Setup views.
     */
    void setupViews();

    /**
     * @brief Connect to data provider
     */
    void connect();

    /**
     * @brief Disconnect the connection
     */
    void disconnect();

    /**
     * @brief Show message box with custom message;
     * @param _message
     */
    void showErrorMessage(const QString& _message);

    /**
     * @brief Open an AEDAT 3.1 file.
     */
    void openFile();

    /**
     * @brief clearViews
     */
    void clearViews();

    void updateGUI();

    void startRecording(const iness::aedat::AedatSourceInfo& _source_info,
                        const std::vector<iness::serialization::AedatType>& _types_to_record = std::vector<iness::serialization::AedatType>(),
                        const QString _filename = QString());

    void stopRecording();

    void updateProgress(int64_t _time);

    // Start input data processing.
    void startInputDataProcessing();

    // Stop input data processing.
    void stopInputDataProcessing();

    // Waits until new input data is available.
    bool inputDataAvailableBlocking();

    // Input data processing thread function.
    void inputDataProcessingLoop();

    // Visualizes event data in a packet.
    void handleEventPacketVisualization(std::shared_ptr<aedat::IEventPacket> _event_packet);

    //! Visualizes a single event
    void visualizeEvent( iness::PolarityEvent& _event, iness::PolarityEventPacket& _packet);

    // Visualizes a single event.
    void handleEventVisualization(std::shared_ptr<aedat::IEventPacket> _event_packet, unsigned int event_index);

    // Distribute event data to recording.
    void handleEventPacketRecording(std::shared_ptr<aedat::IEventPacket> _event_packet, double utc = 0.0);

    // Callback for data producers (driver or network).
    void eventPacketProducerCallback(std::shared_ptr<iness::aedat::IEventPacket> _event_packet);

protected:
    void closeEvent(QCloseEvent *_event) override;


    //members
private:
    Ui::MainWindow *ui;
    QLabel *status_label_;
    VideoControls* video_controls_;
    DeviceControls* device_controls_;
    std::ofstream utc_out;

    // status flags
    SeesControlState state_;

    enum class DataSourceType
    {
        USB_DEVICE,
        NETWORK_DEVICE
    };

    DataSourceType data_source_type_;
    bool is_source_controls_open_ = false;
    bool is_recording_file_defined_ = false;

    QString filename_;

    std::shared_ptr<DataView> data_view_;

    iness::aedat::AedatOutput output_;
    iness::aedat::AedatInput input_;

    std::mutex data_input_buffer_mutex_;
    std::condition_variable data_input_buffer_cv_;
    std::atomic<bool> run_input_data_processing_thread_;
    std::queue<std::shared_ptr<aedat::IEventPacket>> data_input_buffer_;
    std::deque<double> data_input_utc;
    std::thread input_data_processing_thread_;

    std::atomic<bool> recording_failed_is_processing_; //!< Used to prevent onRecordingFailed() slot to be called again before returning.

    static const unsigned int desactivate_display_buffer_size_; //!< When the data_input_buffer_ reaches this size, the display is stopped to accelerate processing.

signals:
    void progressChanged(long);
    void sourceConnected(bool);
    void recordingFailed();

public slots:
    /**
     * @brief Handle disconnect event.
     * @param _message
     */
    void onDisconnect(QString _message);

    /**
     * @brief Event callback to handle closing event.
     */
    void on_actionClose_triggered();

    /**
     * @brief Event callback to handle version info.
     */
    void on_actionAbout_triggered();

    /**
     * @brief Event callback to handle connect button click.
     */
    void on_actionConnect_triggered();
    void on_actionRecord_triggered();

    void onRemoteOpenDevice();
    void onRemoteCloseDevice();
    void onRemoteStartRecording(QString _filename, std::vector<iness::serialization::AedatType> _packet_types);
    void onRemoteStopRecording();
    void onRemoteDeviceTimeReset();

private slots:
    void on_actionOpen_triggered();
    void change_source(QString _source);

    void playerPlay();
    void playerPause();
    void playerStop();
    void playerStepForward();
    void playerStepBackward();
    void playerRepeat(bool _checked);
    void onSetSpeed(double _speed);
    void onChangeProgress(long _timestamp);
    void onChangedEventsPerFrame(unsigned int _events_per_frame);
    void on_actionConfigure_triggered(bool _checked);
    void onRecordingFailed();
    void onSourceConfigureDialogClosed();
};

#endif // MAINWINDOW_H
