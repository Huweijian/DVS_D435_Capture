/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#ifndef DEVICE_CONTROLS_HPP
#define DEVICE_CONTROLS_HPP

#include <QDialog>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUdpSocket>
#include <QByteArray>
#include <QVBoxLayout>
#include <QSpinBox>

#include <list>

#include <iness_common/device/sees/sees.hpp>

namespace remote_control
{
// command struct
struct command
{
    QString str_; // command string
    int arg_; // number of arguments + 1
};

// command definitions
const command list_commands = {"?", 1};
const command start_recording = {"startRecording", 2};
const command stop_recording = {"stopRecording", 1};
const command get_software_info = {"getSoftwareInfo", 1};
const command get_device_info = {"getDeviceInfo", 1};
const command open_device = {"openDevice", 1};
const command close_device = {"closeDevice", 1};
const command load_config_file = {"loadConfigurationFile", 2};
const command write_config_file = {"writeConfigurationFile", 2};
const command reset_timestamp = {"resetDeviceTime", 1};
const command set_aps_enabled = {"setApsEnabled", 2};
const command get_aps_enabled = {"getApsEnabled", 1};
const command set_imu_enabled = {"setImuEnabled", 2};
const command get_imu_enabled = {"getImuEnabled", 1};
const command set_dvs_enabled = {"setDvsEnabled", 2};
const command get_dvs_enabled = {"getDvsEnabled", 1};
const command set_exposure = {"setExposure", 2};
const command get_exposure = {"getExposure", 1};
const command set_expert_device_config = {"setExpertDeviceConfig", 3};
const command get_expert_device_config = {"getExpertDeviceConfig", 2};

const std::list<command> command_list = {start_recording, stop_recording, get_software_info, get_device_info, open_device, close_device,
                                         load_config_file, write_config_file, reset_timestamp, set_aps_enabled, get_aps_enabled,
                                         set_imu_enabled, get_imu_enabled, set_dvs_enabled, get_dvs_enabled, set_exposure, get_exposure,
                                         set_expert_device_config, get_expert_device_config};

// return values
const QString return_success = "0";
const QString return_failed = "-1";

// error codes
enum error_code {
    error_code_none,
    error_code_unknown_command,
    error_code_not_supported,
    error_code_too_few_arguments,
    error_code_too_many_arguments,
    error_code_device_expired,
    error_code_file_does_not_exist,
    error_code_invalid_arguments
};

// error struct
struct error
{
    error_code code_; // error code (enum)
    QString message_; // error message
};

// error definitions
const std::list<error> error_list = { {error_code_unknown_command, "error=unknown_command"},
                                      {error_code_not_supported, "error=not_supported"},
                                      {error_code_too_few_arguments, "error=too_few_arguments"},
                                      {error_code_too_many_arguments, "error=too_many_arguments"},
                                      {error_code_device_expired, "error=device_expired"},
                                      {error_code_file_does_not_exist, "error=file_does_not_exist"},
                                      {error_code_invalid_arguments, "error=invalid_arguments"} };
}

/**
 * @brief This base class is used to generate custom expert items
 * as used in the expert tree widget
 */
class ExpertItem : public QTreeWidgetItem
{

public:
    ExpertItem(iness::ptree::Node::Ptr _node, QTreeWidget *_tree = 0, int _type = EmptyType);
    ExpertItem(iness::ptree::Node::Ptr _node, ExpertItem *_parent = 0, int _type = EmptyType);
    ~ExpertItem();

    virtual void writeToDevice() {};
    virtual void readFromDevice()
    {
        // iterate through all it's children
        for (int i=0;i<childCount();i++) {
            if (child(i)->type()>=ExpertItem::EmptyType)
                static_cast<ExpertItem*>(child(i))->readFromDevice();
        }
    };

    // item types
    enum ItemType {
        EmptyType = 1000,
        SimpleType,
        FancyType,
        VectorType
    };

    static constexpr int SETTING = 0;
    static constexpr int VALUE = 1;

private:
    std::weak_ptr<iness::ptree::Node> node_;

};

/**
 * @brief Template class for custom expert items that store a simple parameter
 */
template <typename T_TYPE>
class SimpleExpertItem : public ExpertItem
{
public:
    SimpleExpertItem(iness::ptree::Node::Ptr _node, ExpertItem *_parent = 0);

    void writeToDevice() override;
    void readFromDevice() override;

private:
    typename std::weak_ptr<iness::ptree::Simple<T_TYPE>> node_;

    T_TYPE getItemValue(bool &_ok);
    void setItemValue(T_TYPE _value);
};

namespace Ui {
class DeviceControls;
}

/**
 * @brief The DeviceControls class
 */
class DeviceControls : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceControls(QWidget *parent = 0);
    ~DeviceControls();

    void setDeviceHandle(std::weak_ptr<iness::device::Sees> _device);
    void removeDeviceHandle();

private slots:
    void on_actionExpert_triggered(bool _checked);

    void on_writeConfigFile_clicked();

    void on_actionCloseDialog_triggered();

    void on_expert_tree_itemChanged(QTreeWidgetItem *_item, int _column);

    void on_expert_tree_itemDoubleClicked(QTreeWidgetItem *_item, int _column);

    void on_start_server_button_clicked(bool _checked);

    void on_server_log_textChanged();

    void readPendingDatagrams();

    void on_expert_tree_itemExpanded(QTreeWidgetItem *);

    void on_load_selected_button_clicked();

    void on_load_all_button_clicked();

    void on_loadConfigFile_clicked();

signals:
    void remoteStartRecording(QString _filename, std::vector<iness::serialization::AedatType> _packet_types);
    void remoteStopRecording();
    void remoteOpenDevice();
    void remoteCloseDevice();
    void remoteResetDeviceTime();
    
private:
    void initUserSettings();
    void clearUserSettings();
    void processDatagram(QByteArray _datagram, QByteArray &_response);
    void populateExpertTreeRecursively(iness::ptree::Node::Ptr _node, ExpertItem *_item = 0);
    bool writeSimpleParameterFromString(iness::ptree::Node::Ptr _node, QString _param);

    void setupGuiParameterTree();

    template <class T_TYPE>
    bool readSimpleParameter(QStringList &_list, iness::ptree::Node::Ptr _entry_node, T_TYPE &_val);

private:
    Ui::DeviceControls *ui;
    
    QVBoxLayout* user_settings_layout_;
    std::list< std::shared_ptr<QWidget> > user_settings_widgets_;

    // device pointer
    std::weak_ptr<iness::device::Sees> device_;

    // remote server
    std::shared_ptr<QUdpSocket> udp_socket_;
    bool is_server_running_ = false;

    // expert
    iness::ptree::Node::Ptr ptree_settings_;
    bool is_expert_tree_loaded_ = false;

    // parameter tree
    iness::ptree::Node::Ptr getParameterNodePointer(QStringList &_list, iness::ptree::Node::Ptr _entry_node);
    QString readSimpleParameterAsString(iness::ptree::Node::Ptr _node);
};


template<class T_PARENT>
class BetterSpinbox: public T_PARENT
{
public:
    BetterSpinbox() = default;
    ~BetterSpinbox() = default;

protected:
    // Overloaded to have editingFinished triggered when pushing up and down buttons
    virtual void mousePressEvent(QMouseEvent* _event) override
    {
        T_PARENT::mousePressEvent(_event);
        T_PARENT::editingFinished();
    }
};

class BetterQSpinBox: public BetterSpinbox<QSpinBox>
{
    Q_OBJECT
};

class BetterQDoubleSpinBox: public BetterSpinbox<QDoubleSpinBox>
{
    Q_OBJECT
};

#endif // DEVICE_CONTROLS_HPP
