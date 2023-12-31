#ifndef BLUETOOTH_CONTROLLER_H
#define BLUETOOTH_CONTROLLER_H

#include <QObject>
#include <QLowEnergyController>
#include "bluetooth_scaner.h"
#include "bluetooth_service.h"
#include <QtBluetooth/QLowEnergyDescriptor>                                               //ble 描述符
#include <QtBluetooth/QLowEnergyService>                                                  //ble 服务
#include <QtBluetooth/QLowEnergyCharacteristic>                                           //ble特性

const std::string WRITE_SERVER_UUID = "0000ffe5-0000-1000-8000-00805f9b34fb";             //65509
const std::string WRITE_CHARACTERISTIC_UUID = "0000ffe9-0000-1000-8000-00805f9b34fb";     //65513
const std::string NOTIFY_SERVER_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb";            //65504
const std::string NOTIFY_CHARACTERISTIC_UUID = "0000ffe4-0000-1000-8000-00805f9b34fb";    //65508


class bluetooth_controller : public QObject
{
    Q_OBJECT
public:
    explicit bluetooth_controller(QObject *parent = nullptr);
    ~bluetooth_controller();
   
    void create_ble_controller();
    void discover_devices();
    void connect_device();
    void discover_services();
    void connect_write_service();
    void connect_notify_service();
    // 向蓝牙设备写入数据
    void write(const QByteArray &data);
    void test_send();

private:
    // 发现服务相关
    void bind_device_slot();
    void bind_write_service_slot();
    void bind_notify_service_slot();
    // 连接设备相关
    void slot_device_connected();                                           // 设备连接成功
    void slot_device_connect_error(QLowEnergyController::Error error);      // 设备连接出现错误
    // 发现服务
    void slot_service_discovered_one(QBluetoothUuid service_uuid);          // 发现一个服务
    void slot_service_discovery_finish();                                   // 服务发现结束
    // 服务相关的
    void slot_write_service_state_changed(QLowEnergyService::ServiceState state);                                          // 监听服务状态变化
    void slot_notify_service_state_changed(QLowEnergyService::ServiceState state);                                          // 监听服务状态变化
    void slot_service_characteristic_changed(QLowEnergyCharacteristic characteristic, QByteArray value);             // 服务的characteristic变化,有数据传来
    void slot_service_error(QLowEnergyService::ServiceError error);                                                  // 错误处理
    void slot_service_descriptor_written(const QLowEnergyDescriptor &descriptor, const QByteArray &value);        // 描述符成功被写
    void slot_service_character_written(const QLowEnergyCharacteristic &characteristic, const QByteArray &value); // 发送成功
    void slot_service_recv_data(const QLowEnergyCharacteristic &c, const QByteArray &value);       

private:
    
    bool device_connected_;                                                 // 设备是否连接
    bluetooth_scaner* ble_scaner_;                                          // 蓝牙扫描对象
    bluetooth_service* bluetooth_service_;                                  // 数据服务对象
    QLowEnergyController* ble_controller_;                                  // 低功耗蓝牙控制
    QBluetoothUuid wirte_service_uuid_;                                     // 服务uuid
    QBluetoothUuid notify_service_uuid_;                                    // 服务uuid
    QLowEnergyService *ble_write_service_;                                  // 低功耗蓝牙服务
    QLowEnergyService *ble_notify_service_;                                 // 低功耗蓝牙服务
    QLowEnergyCharacteristic read_characteristic_;                          // 读特征值
    QLowEnergyCharacteristic write_characteristic_;                         // 写特征值
    QLowEnergyCharacteristic notify_characteristic_;                        // 通知特征值
    QLowEnergyService::WriteMode write_mode_;                               // 写入模式

signals:

};

#endif // BLUETOOTH_CONTROLLER_H
