#ifndef BLUETOOTH_SERVICE_H
#define BLUETOOTH_SERVICE_H

#include <QObject>
#include <QtBluetooth/QLowEnergyDescriptor>                             //ble 描述符
#include <QtBluetooth/QLowEnergyService>                                //ble 服务
#include <QtBluetooth/QLowEnergyCharacteristic>                         //ble特性

#define CHUNK_SIZE 20

class bluetooth_service : public QObject
{
    Q_OBJECT
public:
    // 构造发送服务
    bluetooth_service();
    ~bluetooth_service();

public:
    // 服务相关的
    void slot_service_state_changed(QLowEnergyService::ServiceState state);                                          // 监听服务状态变化
    void slot_service_characteristic_changed(QLowEnergyCharacteristic characteristic, QByteArray value);             // 服务的characteristic变化,有数据传来
    void slot_service_error(QLowEnergyService::ServiceError error);                                                  // 错误处理
    void slot_service_descriptor_written(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);        // 描述符成功被写
    void slot_service_character_written(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue); // 发送成功
    void slot_service_recv_data(const QLowEnergyCharacteristic &c, const QByteArray &value);                         // 收到蓝牙设备的数据

private:
    QLowEnergyCharacteristic read_characteristic_;                      // 读特征值
    QLowEnergyCharacteristic write_characteristic_;                     // 写特征值
    QLowEnergyCharacteristic notify_characteristic_;                    // 通知特征值
    QLowEnergyService::WriteMode write_mode_;                           // 写入模式
    QLowEnergyService *ble_write_service_;


signals:

};

#endif // BLUETOOTH_SERVICE_H
