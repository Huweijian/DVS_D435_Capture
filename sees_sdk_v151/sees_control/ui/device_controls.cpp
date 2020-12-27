/*
 * Copyright (C) Insightness AG, Switzerland - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Marc Osswald <marc@insightness.com>
 * Wed February 2017
 */

#include "device_controls.hpp"
#include "ui_device_controls.h"
#include "versioninfo.hpp"

#include <type_traits>
#include <sstream>

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QScrollBar>
#include <QtNetwork>
#include <QCheckBox>
#include <QSettings>

#include "iness_common/util/type_registry.hpp"
#include "iness_common/util/execute_callable_on_each.hpp"

DeviceControls::DeviceControls(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DeviceControls)
{
    ui->setupUi(this);

    user_settings_layout_ = ui->user_settings;

    // context menu
    this->addAction(ui->actionExpert);
    this->addAction(ui->actionCloseDialog);
    this->setContextMenuPolicy(Qt::ActionsContextMenu);

    // setup expert mode
    ui->expert_frame->hide();
    this->resize(this->minimumSize());

    // setup udp server
    udp_socket_.reset(new QUdpSocket(this));

    // get own ip
    QString ip_string("127.0.0.1");
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            ip_string = address.toString();
    }
    ui->ip_label->setText(ip_string);
    ui->ip_label->setReadOnly(true);


    setupGuiParameterTree();
}

DeviceControls::~DeviceControls()
{
    delete ui;
}

void DeviceControls::setDeviceHandle(std::weak_ptr<iness::device::Sees> _device)
{
    device_ = _device;
    
    initUserSettings();
    setupGuiParameterTree();


}

void DeviceControls::removeDeviceHandle()
{
    // release weak device pointer
    device_.reset();

    // clear expert tree
    ui->expert_tree->clear();
    setupGuiParameterTree();

    // stop remote server
    if (is_server_running_)
        ui->start_server_button->setChecked(false);
    
    clearUserSettings();
}

ExpertItem::ExpertItem(iness::ptree::Node::Ptr _node, QTreeWidget *_tree, int _type): QTreeWidgetItem(_tree,_type)
{
    node_ = _node;
    iness::ptree::Node::Ptr node = node_.lock();
    if (node)
        setText(SETTING,QString::fromStdString(node->name()));
}

ExpertItem::ExpertItem(iness::ptree::Node::Ptr _node, ExpertItem *_parent, int _type) : QTreeWidgetItem(_parent,_type)
{
    node_ = _node;
    iness::ptree::Node::Ptr node = node_.lock();
    if (node)
        setText(SETTING,QString::fromStdString(node->name()));
}

ExpertItem::~ExpertItem()
{
}

template <typename T_TYPE>
SimpleExpertItem<T_TYPE>::SimpleExpertItem(iness::ptree::Node::Ptr _node, ExpertItem *_parent) : ExpertItem(_node,_parent,SimpleType)
{

    node_ = std::static_pointer_cast<iness::ptree::Simple<T_TYPE>>(_node);

    // If the parameter node supports it, register callback function to update the gui element whenever the parameter node is written to
    if(_node->hasPropertyEntry("SupportsOnChangeCallback"))
    {
       _node->addOnChangeCallback([&](){this->readFromDevice();});
    }
    readFromDevice();

    typename iness::ptree::Simple<T_TYPE>::Ptr node = node_.lock();
    if (node)
    {
        if( node->rwType() == iness::ptree::Node::RwType::READ_ONLY)
            setFlags(flags() & ~Qt::ItemFlag::ItemIsEditable & ~Qt::ItemIsSelectable );
        else if (node->typeId() == iness::util::ExtendedTypeRegistry::getId<bool>())
            setFlags(flags() & ~Qt::ItemFlag::ItemIsEditable);
        else if (node->rwType() != iness::ptree::Node::RwType::READ_ONLY)
            setFlags(flags() | Qt::ItemIsEditable);
    }
}

template <typename T_TYPE>
void SimpleExpertItem<T_TYPE>::readFromDevice()
{
    typename iness::ptree::Simple<T_TYPE>::Ptr node = node_.lock();
    if (node)
    {
        T_TYPE val;
        iness::ptree::directRead<T_TYPE>(node,val);
        setItemValue(val);
    }
}

template <typename T_TYPE>
void SimpleExpertItem<T_TYPE>::writeToDevice()
{
    bool ok;
    T_TYPE val = getItemValue(ok);

    if (ok)
    {
        typename iness::ptree::Simple<T_TYPE>::Ptr node = node_.lock();
        if (node)
            iness::ptree::directWrite(node,val);
    }
    else
        QMessageBox::warning(0,QString("Warning"),QString("Invalid parameter value."));
}

template <>
void SimpleExpertItem<bool>::setItemValue(bool _checked)
{
    setCheckState(VALUE,_checked ? Qt::Checked : Qt::Unchecked);
}

// This is called to create a QString for all types for which std::is_integral<TYPE> is true (integers)
template<class T_TYPE, typename std::enable_if< std::is_integral<T_TYPE>::value >::type* = nullptr>
QString toQString( T_TYPE _value, typename iness::ptree::Simple<T_TYPE>::Ptr _node )
{
  if (_node->template hasPropertyEntry("NumberTextFormat") && _node->template getPropertyEntry<std::string>("NumberTextFormat")=="hexadecimal")
    return QString("%1").arg(_value,4,16,QChar('0')).prepend("0x");
  else
    return QString("%1").arg(_value);
}

// This is called to create a QString for all other types
template<class T_TYPE, typename std::enable_if< !std::is_integral<T_TYPE>::value >::type* = nullptr>
QString toQString( T_TYPE _value, typename iness::ptree::Simple<T_TYPE>::Ptr )
{
  std::stringstream converter;
  converter << _value;
  return QString("%1").fromStdString(converter.str());
}

template <typename T_TYPE>
void SimpleExpertItem<T_TYPE>::setItemValue(T_TYPE _value)
{
    typename iness::ptree::Simple<T_TYPE>::Ptr node = node_.lock();
    if (node)
        setText( VALUE, toQString(_value, node) );
};

template <>
bool SimpleExpertItem<bool>::getItemValue(bool &_ok)
{
    _ok = true;
    return (checkState(VALUE) == Qt::Checked ? true : false);
}

template <>
uint32_t SimpleExpertItem<uint32_t>::getItemValue(bool &_ok)
{
    return text(VALUE).toUInt(&_ok,0);
}

template <typename T_TYPE>
T_TYPE SimpleExpertItem<T_TYPE>::getItemValue(bool &_ok)
{
    T_TYPE val;
    std::stringstream converter;
    converter << text(VALUE).toStdString();
    converter >> val;
    _ok = !(converter.rdstate()&std::ifstream::failbit || converter.rdstate()&std::ifstream::badbit);
    return val;
}

template<typename T_TYPE>
class SimpleExpertItemCreator
{
public:
    static void call(iness::ptree::Node::Ptr _node, ExpertItem *_item)
    {
        if( _node->type()==iness::ptree::Node::Type::SIMPLE &&
           _node->as<iness::ptree::SimpleBase>()->typeId() == iness::util::ExtendedTypeRegistry::getId<T_TYPE>() )
        {
            new SimpleExpertItem<T_TYPE>(_node,_item);
        }
    }
};

void DeviceControls::populateExpertTreeRecursively(iness::ptree::Node::Ptr _node, ExpertItem *_item)
{
    // check if child is a leaf
    if (_node->nrOfChildNodes()==0)
    {
        // only support simple parameter types for now
        if( _node->type()==iness::ptree::Node::Type::SIMPLE )
        {
            // This basically calls the "call(...)"-function of SimpleExpertItemCreator with every type in the StdStreamSupportedTypes
            iness::util::ExecuteCallableOnEach<SimpleExpertItemCreator,iness::util::StdStreamSupportedTypes>::call(_node,_item);
            
        }
        // don't support other types (such as vector) for now
        else
        {
            new ExpertItem(_node,_item,ExpertItem::EmptyType);
        }
        return;
    }

    // create a child item
    ExpertItem *new_item = nullptr;
    if (!_node->name().empty())
    {
        new_item = _item ? new ExpertItem(_node,_item) : new ExpertItem(_node,ui->expert_tree);

    }

    // iterate through its children
    for (iness::ptree::Node::Ptr child: *_node)
    {
        populateExpertTreeRecursively(child,new_item);
    }
}

void DeviceControls::on_actionExpert_triggered(bool _checked)
{
    if (_checked)
    {
        // enable expert mode
        ui->actionExpert->setText("Disable expert mode");
        ui->expert_frame->show();
    }
    else
    {
        // disable expert mode
        ui->actionExpert->setText("Enable expert mode");
        ui->expert_frame->hide();
        this->resize(this->minimumSize());
    }
}

void DeviceControls::on_writeConfigFile_clicked()
{
    std::shared_ptr<iness::device::Sees> device = device_.lock();
    if (device)
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),QSettings().value("last_config_file", ".").toString(),tr("YAML (*.yaml *.yml)"));
        if (!fileName.isEmpty()) {
            QSettings().setValue("last_config_file", fileName);
            device->writeConfigurationToFile(fileName.toStdString());
        }
    }
}

void DeviceControls::on_actionCloseDialog_triggered()
{
    this->close();
}

void DeviceControls::on_expert_tree_itemChanged(QTreeWidgetItem *_item, int _column)
{
    if (_column == ExpertItem::VALUE && is_expert_tree_loaded_)
    {
        if (_item->type() == ExpertItem::SimpleType)
            static_cast<ExpertItem*>(_item)->writeToDevice();
    }
}

void DeviceControls::on_expert_tree_itemDoubleClicked(QTreeWidgetItem *_item, int _column)
{
    // make sure that only items in the right column can be edited
    if (_column == ExpertItem::VALUE && (_item->flags() & Qt::ItemIsEditable)) {
        ui->expert_tree->editItem(_item,1);
    }
}

void DeviceControls::on_start_server_button_clicked(bool _checked)
{
    if (_checked)
    {
        if (!is_server_running_)
        {
            // start the server
            ui->server_log->append(QString("%1: Starting remote server.").arg(QTime::currentTime().toString()));
            if (udp_socket_->bind(QHostAddress::Any, ui->port_box->value()))
            {
                ui->server_log->append(QString("%1: Socket bound to port %2. Listening...").arg(QTime::currentTime().toString()).arg(ui->port_box->value()));
                is_server_running_ = true;
                QObject::connect(udp_socket_.get(),SIGNAL(readyRead()),this,SLOT(readPendingDatagrams()));
            }
            else
            {
                ui->server_log->append(QString("%1: Couldn't bind to port %2.").arg(QTime::currentTime().toString()).arg(ui->port_box->value()));
                ui->start_server_button->setChecked(false);
            }
        }
    }
    else
    {
        if (is_server_running_)
        {
            // stop the server
            ui->server_log->append(QString("%1: Stopping remote server.").arg(QTime::currentTime().toString()));
            udp_socket_->close();
            is_server_running_ = false;
        }
    }
    // change the button text
    ui->start_server_button->setText((is_server_running_) ? "Stop" : "Start");
}

void DeviceControls::readPendingDatagrams()
{
    while (udp_socket_->hasPendingDatagrams()) {
        QByteArray datagram, response;
        datagram.resize(udp_socket_->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udp_socket_->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);
        // convert address to ip4
        if (sender.protocol() == QAbstractSocket::IPv6Protocol)
            sender = QHostAddress(sender.toIPv4Address());
        ui->server_log->append(QString("%1: Received remote command from %2:%3.").arg(QTime::currentTime().toString()).arg(sender.toString()).arg(senderPort));
        processDatagram(datagram, response);

        // send response
        if (!response.isEmpty())
        {
            // add newline character
            response.append("\n");
            // send response
            udp_socket_->writeDatagram(response,sender,senderPort);
        }
    }
}

template <class T_TYPE>
bool DeviceControls::readSimpleParameter(QStringList &_list, iness::ptree::Node::Ptr _entry_node, T_TYPE &_val)
{
    // iterate through its children
    for (iness::ptree::Node::Ptr child: *_entry_node)
    {
        if (child->name() == _list.at(0).toStdString())
        {
            // return node
            if (_list.size() == 1)
            {
                iness::ptree::directRead<T_TYPE>(child,_val);
                return true;
            }
            else if (_list.size() > 1)
            {
                _list.removeFirst();
                return readSimpleParameter(_list,child,_val);
            }
        }
    }

    // failed
    return false;
}

iness::ptree::Node::Ptr DeviceControls::getParameterNodePointer(QStringList &_list, iness::ptree::Node::Ptr _entry_node)
{
    // iterate through its children
    for (iness::ptree::Node::Ptr child: *_entry_node)
    {
        if (child->name() == _list.at(0).toStdString())
        {
            // return node
            if (_list.size() == 1)
            {
                return child;
            }
            else if (_list.size() > 1)
            {
                _list.removeFirst();
                return getParameterNodePointer(_list,child);
            }
        }
    }

    // failed
    return nullptr;
}

template<typename T_TYPE>
class SimpleParamReader
{
public:
    static void call(iness::ptree::Node::Ptr _node, QString *_str)
    {
        if( _node->type()==iness::ptree::Node::Type::SIMPLE &&
           _node->as<iness::ptree::SimpleBase>()->typeId() == iness::util::ExtendedTypeRegistry::getId<T_TYPE>() )
        {

            T_TYPE val;
            iness::ptree::directRead<T_TYPE>(_node,val);
            (*_str) = QString(toQString(val,_node->as<iness::ptree::Simple<T_TYPE>>()));
        }
    }
};

QString DeviceControls::readSimpleParameterAsString(iness::ptree::Node::Ptr _node)
{
    QString str = "unknown";
    if (_node)
        iness::util::ExecuteCallableOnEach<SimpleParamReader,iness::util::StdStreamSupportedTypes>::call(_node,&str);
    return str;
}

// This is called to convert a QString to T_TYPE
template<class T_TYPE>
T_TYPE fromQString( QString &, bool &ok)
{
    ok = false;
    return T_TYPE();
}

template<>
uint32_t fromQString(QString &_string, bool &ok)
{
    return _string.toUInt(&ok,0);
}

template<>
bool fromQString(QString &_string, bool &ok)
{
    if (_string == "1" || QString::compare(_string,"true",Qt::CaseInsensitive) == 0)
    {
        ok = true;
        return true;
    }
    else if (_string == "0" || QString::compare(_string,"false",Qt::CaseInsensitive) == 0)
    {
        ok = true;
        return false;
    }
    ok = false;
    return 0;
}

template<typename T_TYPE>
class SimpleParamWriter
{
public:
    static void call(iness::ptree::Node::Ptr _node, QString *_str, bool *_ok)
    {
        if( _node->type()==iness::ptree::Node::Type::SIMPLE &&
           _node->as<iness::ptree::SimpleBase>()->typeId() == iness::util::ExtendedTypeRegistry::getId<T_TYPE>() )
        {
            T_TYPE val = fromQString<T_TYPE>(*_str, *_ok);
            if (*_ok)
                iness::ptree::directWrite<T_TYPE>(_node,val);
        }
    }
};

bool DeviceControls::writeSimpleParameterFromString(iness::ptree::Node::Ptr _node, QString _param)
{
    bool ok = false;
    if (_node)
        iness::util::ExecuteCallableOnEach<SimpleParamWriter,iness::util::StdStreamSupportedTypes>::call(_node,&_param,&ok);
    return ok;
}

void DeviceControls::setupGuiParameterTree()
{
    // clear expert tree if not yet done already
    ui->expert_tree->clear();

    // build a parameter tree for sees control

    ptree_settings_ = iness::ptree::Node::create();

    // populate the GUI with the current settings
    std::shared_ptr<iness::device::Sees> device = device_.lock();
    if (device)
    {
        // populate the expert tree
        (*ptree_settings_).addEntry("Device", device->parameterTree());
        iness::ptree::Node::Ptr device_root = device->parameterTree();
    }
    populateExpertTreeRecursively(ptree_settings_);
    ui->expert_tree->resizeColumnToContents(0);

    // expand items on top level only
    for (int i=0;i<ui->expert_tree->topLevelItemCount();i++)
        ui->expert_tree->topLevelItem(i)->setExpanded(true);

    // expert tree has been loaded
    is_expert_tree_loaded_ = true;
}

void DeviceControls::processDatagram(QByteArray _datagram, QByteArray &_response)
{
    QString command(_datagram);
    QStringList tokens = command.split(" ",QString::SkipEmptyParts);

    remote_control::error_code error = remote_control::error_code_none;

    // list all commands
    if (tokens[0] == remote_control::list_commands.str_)
    {
        for (auto const& c : remote_control::command_list)
        {
            _response.append(QString("%1 <%2>\n").arg(c.str_).arg(c.arg_-1));
        }
    }

    // start recording
    else if (tokens[0] == remote_control::start_recording.str_)
    {
        if (tokens.size() >= remote_control::start_recording.arg_)
        {
            ui->server_log->append(QString("%1: Start recording. Saving to %2").arg(QTime::currentTime().toString()).arg(tokens[1]));

            std::vector<iness::serialization::AedatType> _packet_types;
            for(int token_index = 2; token_index < tokens.size(); ++token_index)
            {
                if(tokens[token_index] == "SPECIAL_EVENT")
                {
                     _packet_types.push_back(iness::serialization::AedatType::SPECIAL_EVENT);
                }
                else if(tokens[token_index] == "POLARITY_EVENT")
                {
                     _packet_types.push_back(iness::serialization::AedatType::POLARITY_EVENT);
                }
                else if(tokens[token_index] == "FRAME_EVENT")
                {
                     _packet_types.push_back(iness::serialization::AedatType::FRAME_EVENT);
                }
                else if(tokens[token_index] == "IMU6_EVENT")
                {
                    _packet_types.push_back(iness::serialization::AedatType::IMU6_EVENT);
                }
                else
                {
                    error = remote_control::error_code_invalid_arguments;
                    break;
                }
            }

            if (error == remote_control::error_code_none)
            {
                emit remoteStartRecording(tokens[1], _packet_types);
                _response.append(QString("%1").arg(remote_control::return_success));
            }
        }
        else
            error = (tokens.size() < remote_control::start_recording.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // stop recording
    else if (tokens[0] == remote_control::stop_recording.str_)
    {
        if (tokens.size() == remote_control::stop_recording.arg_)
        {
            ui->server_log->append(QString("%1: Stop recording.").arg(QTime::currentTime().toString()));
            emit remoteStopRecording();
            _response.append(QString("%1").arg(remote_control::return_success));
        }
        else
            error = (tokens.size() < remote_control::stop_recording.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // get software info
    else if (tokens[0] == remote_control::get_software_info.str_)
    {
        if (tokens.size() == remote_control::get_software_info.arg_)
        {
            _response.append(QString("%1 sees_control_version=%2 libiness_version=%3").arg(remote_control::return_success).arg(version_info::version).arg(LIBINESS_VERSION));
        }
        else
            error = (tokens.size() < remote_control::get_software_info.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // get device info
    else if (tokens[0] == remote_control::get_device_info.str_)
    {
        if (tokens.size() == remote_control::get_device_info.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                // get module name
                QString module = readSimpleParameterAsString(getParameterNodePointer(QStringList() << "Driver" << "Information" << "ModuleType",device->parameterTree()));

                // get serial number
                QString serial = readSimpleParameterAsString(getParameterNodePointer(QStringList() << "Driver" << "Information" << "Device" << "SerialNr",device->parameterTree()));

                // get logic version
                QString  logic = readSimpleParameterAsString(getParameterNodePointer(QStringList() << "Driver" << "Information" << "Device" << "LogicVersion",device->parameterTree()));

                // get driver version
                QString maj = readSimpleParameterAsString(getParameterNodePointer(QStringList() << "Driver" << "Information" << "DriverRelease" << "Major",device->parameterTree()));
                QString min = readSimpleParameterAsString(getParameterNodePointer(QStringList() << "Driver" << "Information" << "DriverRelease" << "Minor",device->parameterTree()));
                QString rev = readSimpleParameterAsString(getParameterNodePointer(QStringList() << "Driver" << "Information" << "DriverRelease" << "Revision",device->parameterTree()));
                QString driver = maj + "." + min + "." + rev;

                // return data
                _response.append(QString("%1 module=%2 serial_number=%3 logic_version=%4 driver_version=%5").arg(remote_control::return_success).arg(module).arg(serial).arg(logic).arg(driver));
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::get_device_info.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // open device
    else if (tokens[0] == remote_control::open_device.str_)
    {
        if (tokens.size() == remote_control::open_device.arg_)
        {
            ui->server_log->append(QString("%1: Open device.").arg(QTime::currentTime().toString()));
            emit remoteOpenDevice();
            _response.append(QString("%1").arg(remote_control::return_success));
        }
        else
            error = (tokens.size() < remote_control::open_device.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // close device
    else if (tokens[0] == remote_control::close_device.str_)
    {
        if (tokens.size() == remote_control::close_device.arg_)
        {
            ui->server_log->append(QString("%1: Close device.").arg(QTime::currentTime().toString()));
            emit remoteCloseDevice();
            _response.append(QString("%1").arg(remote_control::return_success));
        }
        else
            error = (tokens.size() < remote_control::close_device.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // load configuration file
    else if (tokens[0] == remote_control::load_config_file.str_)
    {
        if (tokens.size() == remote_control::load_config_file.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                QFileInfo check_file(tokens[1]);
                if (check_file.exists() && check_file.isFile())
                {
                    device->loadConfiguration(tokens[1].toStdString());
                    _response.append(QString("%1").arg(remote_control::return_success));
                }
                else
                    error = remote_control::error_code_file_does_not_exist;
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::load_config_file.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // write configuration file
    else if (tokens[0] == remote_control::write_config_file.str_)
    {
        if (tokens.size() == remote_control::write_config_file.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                device->writeConfigurationToFile(tokens[1].toStdString());
                _response.append(QString("%1").arg(remote_control::return_success));
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::write_config_file.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // reset device time
    else if (tokens[0] == remote_control::reset_timestamp.str_)
    {
        if (tokens.size() == remote_control::reset_timestamp.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                device->resetDeviceTime();
                _response.append(QString("%1").arg(remote_control::return_success));
                emit remoteResetDeviceTime();
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::reset_timestamp.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;

    }

    // set aps enabled
    else if (tokens[0] == remote_control::set_aps_enabled.str_)
    {
        if (tokens.size() == remote_control::set_aps_enabled.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                if (tokens[1] == "1" || QString::compare(tokens[1],"true",Qt::CaseInsensitive) == 0)
                {
                    device->setApsEnabled(true);
                    _response.append(QString("%1").arg(remote_control::return_success));
                }
                else if (tokens[1] == "0" || QString::compare(tokens[1],"false",Qt::CaseInsensitive) == 0)
                {
                    device->setApsEnabled(false);
                    _response.append(QString("%1").arg(remote_control::return_success));
                }
                else
                    error = remote_control::error_code_invalid_arguments;
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::set_aps_enabled.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // get aps enabled
    else if (tokens[0] == remote_control::get_aps_enabled.str_)
    {
        if (tokens.size() == remote_control::get_aps_enabled.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                bool val = device->getApsEnabled();
                _response.append(QString("%1 %2").arg(remote_control::return_success).arg(val));
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::get_aps_enabled.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // set imu enabled
    else if (tokens[0] == remote_control::set_imu_enabled.str_)
    {
        if (tokens.size() == remote_control::set_imu_enabled.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                if (tokens[1] == "1" || QString::compare(tokens[1],"true",Qt::CaseInsensitive) == 0)
                {
                    device->setImuEnabled(true);
                    _response.append(QString("%1").arg(remote_control::return_success));
                }
                else if (tokens[1] == "0" || QString::compare(tokens[1],"false",Qt::CaseInsensitive) == 0)
                {
                    device->setImuEnabled(false);
                    _response.append(QString("%1").arg(remote_control::return_success));
                }
                else
                    error = remote_control::error_code_invalid_arguments;
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::set_imu_enabled.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // get imu enabled
    else if (tokens[0] == remote_control::get_imu_enabled.str_)
    {
        if (tokens.size() == remote_control::get_imu_enabled.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                bool val = device->getImuEnabled();
                _response.append(QString("%1 %2").arg(remote_control::return_success).arg(val));
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::get_imu_enabled.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // set dvs enabled
    else if (tokens[0] == remote_control::set_dvs_enabled.str_)
    {
        if (tokens.size() == remote_control::set_dvs_enabled.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                if (tokens[1] == "1" || QString::compare(tokens[1],"true",Qt::CaseInsensitive) == 0)
                {
                    device->setDvsEnabled(true);
                    _response.append(QString("%1").arg(remote_control::return_success));
                }
                else if (tokens[1] == "0" || QString::compare(tokens[1],"false",Qt::CaseInsensitive) == 0)
                {
                    device->setDvsEnabled(false);
                    _response.append(QString("%1").arg(remote_control::return_success));
                }
                else
                    error = remote_control::error_code_invalid_arguments;
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::set_dvs_enabled.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // get dvs enabled
    else if (tokens[0] == remote_control::get_dvs_enabled.str_)
    {
        if (tokens.size() == remote_control::get_dvs_enabled.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                bool val = device->getDvsEnabled();
                _response.append(QString("%1 %2").arg(remote_control::return_success).arg(val));
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::get_dvs_enabled.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // set exposure
    else if (tokens[0] == remote_control::set_exposure.str_)
    {
        if (tokens.size() == remote_control::set_exposure.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                bool ok;
                uint exposure = tokens[1].toUInt(&ok);
                if (ok)
                {
                    device->setExposure(exposure);
                    exposure = device->getExposure();
                    _response.append(QString("%1 %2").arg(remote_control::return_success).arg(exposure));
                }
                else
                    error = remote_control::error_code_invalid_arguments;
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::set_exposure.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // get exposure
    else if (tokens[0] == remote_control::get_exposure.str_)
    {
        if (tokens.size() == remote_control::get_exposure.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                uint val = device->getExposure();
                _response.append(QString("%1 %2").arg(remote_control::return_success).arg(val));
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::get_exposure.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // set expert device config
    else if (tokens[0] == remote_control::set_expert_device_config.str_)
    {
        if (tokens.size() == remote_control::set_expert_device_config.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                QStringList node_list = tokens[1].split(".",QString::SkipEmptyParts);
                if (writeSimpleParameterFromString(getParameterNodePointer(node_list,device->parameterTree()),tokens[2]))
                {
                    node_list = tokens[1].split(".",QString::SkipEmptyParts);
                    QString param = readSimpleParameterAsString(getParameterNodePointer(node_list,device->parameterTree()));
                    _response.append(QString("%1 %2").arg(remote_control::return_success).arg(param));
                }
                else
                    error = remote_control::error_code_invalid_arguments;
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::set_expert_device_config.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // get expert device config
    else if (tokens[0] == remote_control::get_expert_device_config.str_)
    {
        if (tokens.size() == remote_control::get_expert_device_config.arg_)
        {
            std::shared_ptr<iness::device::Sees> device = device_.lock();
            if (device)
            {
                QStringList node_list = tokens[1].split(".",QString::SkipEmptyParts);
                QString param = readSimpleParameterAsString(getParameterNodePointer(node_list,device->parameterTree()));
                _response.append(QString("%1 %2").arg(remote_control::return_success).arg(param));
            }
            else
                error = remote_control::error_code_device_expired;
        }
        else
            error = (tokens.size() < remote_control::get_expert_device_config.arg_) ? remote_control::error_code_too_few_arguments : remote_control::error_code_too_many_arguments;
    }

    // unknown command
    else
    {
        error = remote_control::error_code_unknown_command;
    }

    // check for error
    if (error != remote_control::error_code_none)
    {
        for (auto const& e : remote_control::error_list)
        {
            if (error == e.code_)
            {
                QString response = QString("%1 %2").arg(remote_control::return_failed).arg(e.message_);
                _response.append(response);
                ui->server_log->append(QString("%1: %2.").arg(QTime::currentTime().toString()).arg(response));
            }
        }
    }
}

void DeviceControls::on_server_log_textChanged()
{
    QScrollBar *sb = ui->server_log->verticalScrollBar();
    sb->setValue(sb->maximum());
}


void DeviceControls::on_expert_tree_itemExpanded(QTreeWidgetItem *)
{
    ui->expert_tree->resizeColumnToContents(0);
}

void DeviceControls::on_load_selected_button_clicked()
{
    if (is_expert_tree_loaded_)
    {
        QList<QTreeWidgetItem*> list = ui->expert_tree->selectedItems();
        foreach (QTreeWidgetItem *item, list) {
            if (item->type() >= ExpertItem::EmptyType)
                static_cast<ExpertItem*>(item)->readFromDevice();
        }
    }
}

void DeviceControls::on_load_all_button_clicked()
{
    if (is_expert_tree_loaded_)
    {
        for (int i=0;i<ui->expert_tree->topLevelItemCount();i++)
        {
            QTreeWidgetItem *item = ui->expert_tree->topLevelItem(i);
            if (item->type() >= ExpertItem::EmptyType)
                static_cast<ExpertItem*>(item)->readFromDevice();
        }
    }
}

void DeviceControls::on_loadConfigFile_clicked()
{
    std::shared_ptr<iness::device::Sees> device = device_.lock();
    if (device)
    {
        QString fileName = QFileDialog::getOpenFileName(this,tr("Load File"),QSettings().value("last_config_file", ".").toString(),tr("YAML (*.yaml *.yml)"));
        if (!fileName.isEmpty()) {
            QSettings().setValue("last_config_file", fileName);
            device->loadConfiguration(fileName.toStdString());
        }
    }
}

template<typename T_SPIN_BOX_TYPE, typename T_VALUE_TYPE, typename T_SIMPLE_PTR >
void addSpinBoxField( T_SIMPLE_PTR _ptree_element,
                      std::shared_ptr<QGroupBox> _qt_box
             )
{
    auto locked_ptr = _ptree_element.lock();
    
    if(locked_ptr)
    {
        QVBoxLayout* box_layout = static_cast<QVBoxLayout*>(_qt_box->layout());

        auto layout = new QHBoxLayout();
        auto label = new QLabel();

        if( locked_ptr->hasPropertyEntry("ReadableName") )
            label->setText( QString::fromStdString( locked_ptr->template getPropertyEntry<std::string>("ReadableName") + ":" ) );
        else
            label->setText( QString::fromStdString( locked_ptr->name() ) );

        auto spin_box = new T_SPIN_BOX_TYPE();
        spin_box->setMinimumSize(80,0);
        layout->insertWidget(-1, label );
        auto spacer = new QSpacerItem(40,20,QSizePolicy::Expanding);
        layout->addItem( spacer );
        layout->insertWidget(-1, spin_box );

        box_layout->addLayout(layout );

        // Set configuration

        T_VALUE_TYPE max = std::numeric_limits<T_VALUE_TYPE>::max();
        T_VALUE_TYPE min = std::numeric_limits<T_VALUE_TYPE>::lowest();

        if( locked_ptr->hasPropertyEntry("Limits"))
        {
        iness::ptree::read( locked_ptr->properties()->entry("Limits")->entry("Min"), min );
        iness::ptree::read( locked_ptr->properties()->entry("Limits")->entry("Max"), max );
        }
        int int_max(max), int_min(min);
        if( int_max<int_min )
        {
            int_max = std::numeric_limits<int>::max();
            int_min = std::numeric_limits<int>::lowest();
        }
        spin_box->setMinimum(int_min);
        spin_box->setMaximum(int_max);

        if( locked_ptr->hasPropertyEntry("StepSize") )
        {
            spin_box->setSingleStep( locked_ptr->template getPropertyEntry<T_VALUE_TYPE>("StepSize") );
        }

        if( locked_ptr->hasPropertyEntry("Description") )
        {
            label->setToolTip( QString::fromStdString(locked_ptr->template getPropertyEntry<std::string>("Description") ) );
            spin_box->setToolTip( QString::fromStdString(locked_ptr->template getPropertyEntry<std::string>("Description") ) );
        }

        // Initialize value
        T_VALUE_TYPE value;
        iness::ptree::directRead(locked_ptr,value);
        spin_box->setValue(value);

        // Connect editing finished signal
        QObject::connect( spin_box, &T_SPIN_BOX_TYPE::editingFinished, [_ptree_element, spin_box](){ auto locked_ptr = _ptree_element.lock(); if(locked_ptr){ auto new_state = spin_box->value(); locked_ptr->write(new_state); }} );

        // Connect ptree change signal
        if( locked_ptr->hasPropertyEntry("SupportsOnChangeCallback") )
        {
          locked_ptr->addOnChangeCallback(
            [_ptree_element, spin_box]()
            {
              auto locked_ptr = _ptree_element.lock();
              if(locked_ptr)
              {
                T_VALUE_TYPE val;
                locked_ptr->read(val);
                spin_box->setValue(val);
              }
            }
          );
        }
    }
    return;
}

// default for unsupported types
template<typename T_VALUE_TYPE, typename T_SIMPLE_PTR,typename  std::enable_if< !std::is_arithmetic<T_VALUE_TYPE>::value >::type* = nullptr >
void addField( T_SIMPLE_PTR,
                      std::shared_ptr<QGroupBox>
             )
{}

// for all integral types
template<typename T_VALUE_TYPE, typename T_SIMPLE_PTR, typename  std::enable_if< std::is_integral<T_VALUE_TYPE>::value >::type* = nullptr  >
void addField( T_SIMPLE_PTR _ptree_element,
                      std::shared_ptr<QGroupBox> _qt_box
             )
{
    addSpinBoxField<BetterQSpinBox,T_VALUE_TYPE>(_ptree_element,_qt_box);
}

// for all floating point types
template<typename T_VALUE_TYPE, typename T_SIMPLE_PTR,typename  std::enable_if< std::is_floating_point<T_VALUE_TYPE>::value >::type* = nullptr >
void addField( T_SIMPLE_PTR _ptree_element,
                      std::shared_ptr<QGroupBox> _qt_box
             )
{
    addSpinBoxField<BetterQDoubleSpinBox,T_VALUE_TYPE>(_ptree_element,_qt_box);
}

// for boolean
template<>
void addField<bool,std::weak_ptr<iness::ptree::Simple<bool>>>( std::weak_ptr<iness::ptree::Simple<bool>> _ptree_element,
                      std::shared_ptr<QGroupBox> _qt_box
             )
{
    iness::ptree::Simple<bool>::Ptr locked_ptr = _ptree_element.lock();
    
    if(locked_ptr)
    {
        
        QVBoxLayout* box_layout = static_cast<QVBoxLayout*>(_qt_box->layout());

        auto layout = new QHBoxLayout();
        auto checkbox = new QCheckBox();

        if( locked_ptr->hasPropertyEntry("ReadableName") )
            checkbox->setText( QString::fromStdString( locked_ptr->template getPropertyEntry<std::string>("ReadableName") ) );
        else
            checkbox->setText( QString::fromStdString( locked_ptr->name() ) );

        layout->insertWidget(-1, checkbox );
        box_layout->addLayout(layout );

        if( locked_ptr->hasPropertyEntry("Description") )
            checkbox->setToolTip( QString::fromStdString(locked_ptr->template getPropertyEntry<std::string>("Description") ) );

        // Initialize checkbox
        bool is_true;
        locked_ptr->read(is_true);
        checkbox->setChecked(is_true);

        // Connect checkbox change signal
        QObject::connect( checkbox, &QCheckBox::stateChanged, [=](bool _new_state){ iness::ptree::Simple<bool>::Ptr locked_ptr = _ptree_element.lock(); if(locked_ptr){locked_ptr->write(_new_state); }} );

        // Connect ptree change signal
        if( locked_ptr->hasPropertyEntry("SupportsOnChangeCallback") )
            locked_ptr->addOnChangeCallback(
              [=]()
              {
                iness::ptree::Simple<bool>::Ptr locked_ptr = _ptree_element.lock();
                if(locked_ptr)
                {
                  bool is_true;
                  locked_ptr->read(is_true);
                  checkbox->setChecked(is_true);
                }
              }
            );
        
    }

    return;
}

template<typename T_VALUE_TYPE>
struct UserSettingItemAdder
{
    static void call( iness::ptree::Node::Ptr _ptree_element,
                      std::shared_ptr<QGroupBox> _qt_box
    )
    {
        if( _ptree_element->type() == iness::ptree::Node::Type::SIMPLE &&
            _ptree_element->as<iness::ptree::SimpleBase>()->typeId() == iness::util::ExtendedTypeRegistry::getId<T_VALUE_TYPE>()
             )
        {
            std::weak_ptr< iness::ptree::Simple<T_VALUE_TYPE> > ptree_element = _ptree_element->as<iness::ptree::Simple<T_VALUE_TYPE>>();
            addField<T_VALUE_TYPE>( ptree_element, _qt_box );
        }
    }

};

void DeviceControls::initUserSettings()
{
    std::shared_ptr<iness::device::Sees> device_ptr = device_.lock();
    if( device_ptr!=nullptr )
    {
        auto ptree = device_ptr->parameterTree();
        if( ptree->hasEntry("UserSettings") )
        {
            iness::ptree::Node::Ptr user_settings = ptree->entry("UserSettings");
            
            for( auto child_ptr: *user_settings )
            {
                std::string config_target_name = child_ptr->name();
                auto config_target_widget = std::make_shared<QGroupBox>();
                auto v_box_layout = new QVBoxLayout();
                config_target_widget->setLayout( v_box_layout );
                
                config_target_widget->setTitle( QString::fromStdString(config_target_name) );

                for( auto member_ptr: *child_ptr )
                    iness::util::ExecuteCallableOnEach<UserSettingItemAdder, iness::util::ExtendedTypeRegistry>::call(member_ptr,config_target_widget);
                
                user_settings_widgets_.push_back(config_target_widget);
                user_settings_layout_->insertWidget( -1, config_target_widget.get() );
            }
        }
    }
}


void DeviceControls::clearUserSettings()
{
    if(user_settings_layout_!=nullptr)
    {
        for( auto widget_ptr: user_settings_widgets_ )
        {
            user_settings_layout_->removeWidget(widget_ptr.get());
        }
    }
    user_settings_widgets_.clear();
}
