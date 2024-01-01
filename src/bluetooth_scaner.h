#ifndef BLUETOOTH_SCANER_H
#define BLUETOOTH_SCANER_H

#include <QObject>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent> //发现设备
#include <QtBluetooth/QBluetoothDeviceInfo>           //设备信息

class bluetooth_scaner : public QObject
{
    Q_OBJECT
public:
    explicit bluetooth_scaner(QObject *parent = nullptr);
    ~bluetooth_scaner();
    void scan_bluetooth();
    bool is_device_found();
    void set_device_name(std::string name);
    void set_device_mac(std::string mac);
    void set_device_address(std::string address);
    QBluetoothDeviceInfo get_device_info();


private:
    void bind_singal_and_slot();
    void create_bluetooth_agent();
    // 发现设备相关
    void slot_device_discovered_one(const QBluetoothDeviceInfo &dev_info);          // 发现一个设备
    void slot_device_discovery_error(QBluetoothDeviceDiscoveryAgent::Error error); // 设备发现出现错误
    void slot_device_discovery_finish();                                           // 设备发现结束
    void slot_device_cancle_scan();                                                // 取消扫描

private:
    QBluetoothDeviceDiscoveryAgent *device_discovery_agent_;                        // 设备发现代理
    QBluetoothDeviceInfo device_info_;                                              // 设备信息
    bool found_device_;                                                             // 是否找到设备
    std::string device_address_;                                                    // 设备地址
    std::string device_name_;                                                       // 设备名称
    std::string device_mac_;                                                        // 设备MAC，可能有用
signals:

};

#endif // BLUETOOTH_SCANER_H
