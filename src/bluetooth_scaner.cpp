#include "bluetooth_scaner.h"
#include <QDebug>
#include <QtBluetooth/QBluetoothUuid>                 //蓝牙uuid
#include <QtBluetooth/QLowEnergyController>           //ble controller

bluetooth_scaner::bluetooth_scaner(QObject *parent) : QObject(parent)
{
    device_name_ = "Tv231u-C3DDCD39";
    create_bluetooth_agent();
    bind_singal_and_slot();
}

bluetooth_scaner::~bluetooth_scaner()
{

}

void bluetooth_scaner::scan_bluetooth()
{
    // 以LowEnergyMethod进行搜索,搜索低功耗蓝牙
    device_discovery_agent_->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void bluetooth_scaner::set_device_name(std::string name)
{
    device_name_ = name;
}

bool bluetooth_scaner::is_device_found()
{
    return found_device_;
}

void bluetooth_scaner::set_device_mac(std::string mac)
{
    device_mac_ = mac;
}

void bluetooth_scaner::set_device_address(std::string address)
{
    device_address_ = address;
}

QBluetoothDeviceInfo bluetooth_scaner::get_device_info()
{
    return device_info_;
}

void bluetooth_scaner::create_bluetooth_agent()
{
    device_discovery_agent_ = new QBluetoothDeviceDiscoveryAgent();     // 创建对象
    device_discovery_agent_->setLowEnergyDiscoveryTimeout(0);           // 一直搜索找到为止
    found_device_ = false;
}

void bluetooth_scaner::bind_singal_and_slot()
{
    void (QBluetoothDeviceDiscoveryAgent::*deviceDiscoveryErrorOccurred)(QBluetoothDeviceDiscoveryAgent::Error) = &QBluetoothDeviceDiscoveryAgent::error; // 有重载
    connect(device_discovery_agent_, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &bluetooth_scaner::slot_device_discovered_one); // 发现了一个设备
    connect(device_discovery_agent_, deviceDiscoveryErrorOccurred, this, &bluetooth_scaner::slot_device_discovery_error); // 设备发现出现错误
    connect(device_discovery_agent_, &QBluetoothDeviceDiscoveryAgent::finished, this, &bluetooth_scaner::slot_device_discovery_finish); // 设备发现结束
    connect(device_discovery_agent_, &QBluetoothDeviceDiscoveryAgent::canceled, this, &bluetooth_scaner::slot_device_cancle_scan); // 设备发现结束
}


void bluetooth_scaner::slot_device_discovered_one(const QBluetoothDeviceInfo &dev_info)
{
    // 名称不为空且是低功耗蓝牙,则考虑加进去
    if (dev_info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        qDebug() << "LowEnergyCoreConfiguration device name:" << dev_info.name()
                 << " address:" << dev_info.address()
                 << " uuid:" << dev_info.deviceUuid();
        if (0 == device_name_.compare(dev_info.name().toStdString()))
        {
             qDebug() << "found required device = " << dev_info.name();
            device_info_ = dev_info;
            found_device_ = true;
            device_discovery_agent_->stop();
        }
    }
}

void bluetooth_scaner::slot_device_discovery_error(QBluetoothDeviceDiscoveryAgent::Error error)
{
    qDebug() << "device discovery error...";
}

void bluetooth_scaner::slot_device_discovery_finish()
{
    if (!found_device_)
    {
        qDebug()<<"not found device over...";
        return;
    }
    qDebug()<< "slot_device_discovery_finish...";
}

void bluetooth_scaner::slot_device_cancle_scan()
{
    qDebug()<<"device found stop scan...";
}


