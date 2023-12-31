#include "bluetooth_controller.h"
#include"bluetooth_service.h"
#include<QDebug>
#include <QTimer>

bluetooth_controller::bluetooth_controller(QObject *parent) : QObject(parent)
{
    ble_scaner_ = new bluetooth_scaner(this);
    ble_controller_ = nullptr;
    device_connected_ = false;
    ble_write_service_ = nullptr;
    ble_notify_service_ = nullptr;
    // notify_service_uuid_ = QBluetoothUuid(QString::fromStdString(NOTIFY_SERVER_UUID));
    // wirte_service_uuid_ = QBluetoothUuid(QString::fromStdString(WRITE_SERVER_UUID));
}

bluetooth_controller::~bluetooth_controller()
{
    delete ble_controller_;
    ble_controller_ = nullptr;
    delete ble_write_service_;
    ble_write_service_ = nullptr;
    delete ble_notify_service_;
    ble_notify_service_ = nullptr;
}

void bluetooth_controller::bind_device_slot()
{
    // bleController的槽函数
    // 连接设备
    connect(ble_controller_, &QLowEnergyController::connected, this, &bluetooth_controller::slot_device_connected);                 // 设备连接成功
    void (QLowEnergyController::*bleDeviceConnectionErrorOccurred)(QLowEnergyController::Error) = &QLowEnergyController::error;     // 有重载
    connect(ble_controller_, bleDeviceConnectionErrorOccurred, this, &bluetooth_controller::slot_device_connect_error);             // 设备连接出现错误
    // 发现服务
    connect(ble_controller_, &QLowEnergyController::serviceDiscovered, this, &bluetooth_controller::slot_service_discovered_one);   // 发现一个服务
    connect(ble_controller_, &QLowEnergyController::discoveryFinished, this, &bluetooth_controller::slot_service_discovery_finish); // 服务发现结束
}

void bluetooth_controller::bind_write_service_slot()
{
    // 监听服务状态变化
    connect(ble_write_service_, &QLowEnergyService::stateChanged, this, &bluetooth_controller::slot_write_service_state_changed);
    // 服务的characteristic变化,有数据传来
    connect(ble_write_service_, &QLowEnergyService::characteristicChanged, this, &bluetooth_controller::slot_service_characteristic_changed);
    // 错误处理
    void (QLowEnergyService::*bleServiceErrorOccurred)(QLowEnergyService::ServiceError) = &QLowEnergyService::error; // 有重载
    connect(ble_write_service_, bleServiceErrorOccurred, this, &bluetooth_controller::slot_service_error);
    // 描述符成功被写
    connect(ble_write_service_, &QLowEnergyService::descriptorWritten, this, &bluetooth_controller::slot_service_descriptor_written);
    //
    connect(ble_write_service_, &QLowEnergyService::characteristicWritten, this, &bluetooth_controller::slot_service_character_written);
}

void bluetooth_controller::bind_notify_service_slot()
{
    // 监听服务状态变化
    connect(ble_notify_service_, &QLowEnergyService::stateChanged, this, &bluetooth_controller::slot_notify_service_state_changed);
    // 服务的characteristic变化,有数据传来
    connect(ble_notify_service_, &QLowEnergyService::characteristicChanged, this, &bluetooth_controller::slot_service_characteristic_changed);
    // 错误处理
    void (QLowEnergyService::*bleServiceErrorOccurred)(QLowEnergyService::ServiceError) = &QLowEnergyService::error; // 有重载
    connect(ble_notify_service_, bleServiceErrorOccurred, this, &bluetooth_controller::slot_service_error);
    connect(ble_notify_service_, &QLowEnergyService::characteristicRead, this, &bluetooth_controller::slot_service_recv_data);

}

void bluetooth_controller::slot_device_connected()
{
    qDebug() << "device connected success...";
    device_connected_ = true;
    qDebug() << "begin discover service...";
    discover_services();
}

void bluetooth_controller::slot_device_connect_error(QLowEnergyController::Error error)
{
    qDebug() << "connect device error..." << error;
}

void bluetooth_controller::create_ble_controller()
{
    qDebug() << "device_info... "<< ble_scaner_->get_device_info().address();
    ble_controller_ = QLowEnergyController::createCentral(ble_scaner_->get_device_info()); // central相当于是主机
    if (ble_controller_) 
    {
        bind_device_slot();
    }
    else
    {
        qDebug() << "create controller failed...";
    }
}

void bluetooth_controller::discover_devices()
{
    ble_scaner_->scan_bluetooth();
}

void bluetooth_controller::connect_device()
{
    if(ble_scaner_->is_device_found())
    {
        ble_controller_->connectToDevice(); // 连接设备
    }
    else
    {
        qDebug()<<"can not found device...";
    }
}

void bluetooth_controller::discover_services()
{
    ble_controller_->discoverServices(); // 开始搜索服务
}

// 发现一个服务
void bluetooth_controller::slot_service_discovered_one(QBluetoothUuid service_uuid)
{
    qDebug() << "found one service uuid =  "
        << service_uuid.toString()
        << "toUint = " 
        << service_uuid.toUInt32();

    if (service_uuid == QBluetoothUuid(QString::fromStdString(WRITE_SERVER_UUID)))
    {
        qDebug() << "found write service uuid = " << service_uuid.toString();
        wirte_service_uuid_ = service_uuid;
         QTimer::singleShot(1000, [=](){
            this->connect_write_service();
        });

         QTimer::singleShot(10000, [=](){
            this->test_send();
        });
    }
    if (service_uuid == QBluetoothUuid(QString::fromStdString(NOTIFY_SERVER_UUID)))
    {
        qDebug() << "found notify service uuid = " << service_uuid.toString();
        notify_service_uuid_ = service_uuid;
         // 读取服务详情 连接服务
        QTimer::singleShot(1000, [=](){
            this->connect_notify_service();
        });
    }
}

// 服务搜索结束
void bluetooth_controller::slot_service_discovery_finish()
{
    qDebug() << "discover service finish...";
}

void bluetooth_controller::connect_write_service()
{
    ble_write_service_ = ble_controller_->createServiceObject(wirte_service_uuid_, this);
    if (ble_write_service_)
    {
        bind_write_service_slot();
        // 触发服务详情发现函数
        ble_write_service_->discoverDetails();
    }
    else
    {
        qDebug() << "create service object failed sevice_id is ..."<< wirte_service_uuid_.toString();
    }
}

void bluetooth_controller::connect_notify_service()
{
    ble_notify_service_ = ble_controller_->createServiceObject(notify_service_uuid_);
    if (ble_notify_service_)
    {
        bind_notify_service_slot();
        // 触发服务详情发现函数
        ble_notify_service_->discoverDetails();
    }
    else
    {
        qDebug() << "create service object failed sevice_id is ..."<< notify_service_uuid_.toString();
    }
}

// 服务状态变化
void bluetooth_controller::slot_write_service_state_changed(QLowEnergyService::ServiceState state) // 服务状态改变
{
    if (QLowEnergyService::InvalidService == state)
    {
        qDebug() << "state is InvalidService...";
    }
    if (QLowEnergyService::DiscoveryRequired == state)
    {
        qDebug() << "state is DiscoveryRequired...";
    }
    if (QLowEnergyService::DiscoveringServices == state)
    {
        qDebug() << "state is DiscoveringServices..";
    }
    if (QLowEnergyService::LocalService == state)
    {
        qDebug() << "state is LocalService...";
    }
    if (QLowEnergyService::ServiceDiscovered == state) // 发现服务
    {
        qDebug() << "state is ServiceDiscovered...";
        QList<QLowEnergyCharacteristic> list = ble_write_service_->characteristics();
        qDebug() << "QLowEnergyCharacteristic list count = " << list.count();
        bool found_valid_descriptor = false;
        for (int i = 0; i < list.count(); i++)
        {
            // 当前位置的bleCharacteritic
            QLowEnergyCharacteristic cur_character = list.at(i);
            // 如果当前characteristic有效
            if (cur_character.isValid())
            {
                qDebug() << "cur_character (" << i << ") is valid, \n name = " 
                    << cur_character.name()
                    << "\n property = " << cur_character.properties()
                    << "\n uuid = " << cur_character.uuid().toString()
                    << "\n isvalid = " << cur_character.isValid()
                    << "\n value = " << cur_character.value().data()
                    << "\n descriptors count = " << cur_character.descriptors().count();

                QList<QLowEnergyDescriptor> desc_list = cur_character.descriptors();
                for (int d = 0; d < desc_list.count(); d++)
                {
                    qDebug() << "descriptor[" << d << "]: name = " << desc_list.at(d).name()
                        << "\ntype = " << desc_list.at(d).type()
                        << "\nuuid = " << desc_list.at(d).uuid().toString()
                        << "\nvalue = " << desc_list.at(d).value().data();
                }

                if (cur_character.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                {
                    write_characteristic_ = cur_character;
                    write_mode_ = QLowEnergyService::WriteWithoutResponse;
                    qDebug() << "this characteristic has WriteNoResponse property" << cur_character.uuid().toString();
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Write)
                {
                    write_characteristic_ = cur_character;
                    write_mode_ = QLowEnergyService::WriteWithResponse;
                    qDebug() << "this characteristic has Write property" << cur_character.uuid().toString();
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Read)
                {
                    qDebug() << "this characteristic has Read property" << cur_character.uuid().toString();
                    read_characteristic_ = cur_character;
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Notify)
                {
                    qDebug() << "this characteristic has Notify property" << cur_character.uuid().toString();
                    notify_characteristic_ = cur_character;
                }

                // 描述符定义特征如何由特定客户端配置
                QLowEnergyDescriptor descriptor = cur_character.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                qDebug()<<"descriptor... "<< descriptor.name();
                if (descriptor.isValid())
                {
                    qDebug() << "this descriptor (" << i << ") is valid";
                    // 0100 enable notification  0000 disable notification
                    ble_write_service_->writeDescriptor(descriptor, QByteArray::fromHex("0100")); 
                    found_valid_descriptor = true;
                }
            }
        } // end for list

        if (!found_valid_descriptor)
        {
            qDebug() <<"found valid descriptor error...";
        }

    } // end 发现服务
}

// 服务状态变化
void bluetooth_controller::slot_notify_service_state_changed(QLowEnergyService::ServiceState state) // 服务状态改变
{
    if (QLowEnergyService::InvalidService == state)
    {
        qDebug() << "state is InvalidService...";
    }
    if (QLowEnergyService::DiscoveryRequired == state)
    {
        qDebug() << "state is DiscoveryRequired...";
    }
    if (QLowEnergyService::DiscoveringServices == state)
    {
        qDebug() << "state is DiscoveringServices..";
    }
    if (QLowEnergyService::LocalService == state)
    {
        qDebug() << "state is LocalService...";
    }
    if (QLowEnergyService::ServiceDiscovered == state) // 发现服务
    {
        qDebug() << "state is ServiceDiscovered...";
        QList<QLowEnergyCharacteristic> list = ble_notify_service_->characteristics();
        qDebug() << "QLowEnergyCharacteristic list count = " << list.count();
        bool found_valid_descriptor = false;
        for (int i = 0; i < list.count(); i++)
        {
            // 当前位置的bleCharacteritic
            QLowEnergyCharacteristic cur_character = list.at(i);
            // 如果当前characteristic有效
            if (cur_character.isValid())
            {
                qDebug() << "cur_character (" << i << ") is valid, \n name = " << cur_character.name()
                         << "\n property = " << cur_character.properties()
                         << "\n uuid = " << cur_character.uuid().toString()
                         << "\n isvalid = " << cur_character.isValid()
                         << "\n value = " << cur_character.value().data()
                         << "\n descriptors count = " << cur_character.descriptors().count();

                QList<QLowEnergyDescriptor> desc_list = cur_character.descriptors();
                for (int d = 0; d < desc_list.count(); d++)
                {
                    qDebug() << "descriptor[" << d << "]: name = " << desc_list.at(d).name()
                             << "\ntype = " << desc_list.at(d).type()
                             << "\nuuid = " << desc_list.at(d).uuid().toString()
                             << "\nvalue = " << desc_list.at(d).value().data();
                }

                if (cur_character.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                {
                    write_characteristic_ = cur_character;
                    write_mode_ = QLowEnergyService::WriteWithoutResponse;
                    qDebug() << "this characteristic has WriteNoResponse property" << cur_character.uuid().toString();
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Write)
                {
                    write_characteristic_ = cur_character;
                    write_mode_ = QLowEnergyService::WriteWithResponse;
                    qDebug() << "this characteristic has Write property" << cur_character.uuid().toString();
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Read)
                {
                    qDebug() << "this characteristic has Read property" << cur_character.uuid().toString();
                    read_characteristic_ = cur_character;
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Notify)
                {
                    qDebug() << "this characteristic has Notify property" << cur_character.uuid().toString();
                    notify_characteristic_ = cur_character;
                }

                // 描述符定义特征如何由特定客户端配置
                QLowEnergyDescriptor descriptor = cur_character.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                qDebug()<<"descriptor... "<< descriptor.name();
                if (descriptor.isValid())
                {
                    qDebug() << "this descriptor (" << i << ") is valid";
                    // 0100 enable notification  0000 disable notification
                    ble_notify_service_->writeDescriptor(descriptor, QByteArray::fromHex("0100")); 
                    found_valid_descriptor = true;
                }
            }
        } // end for list

        if (!found_valid_descriptor)
        {
            qDebug() <<"found valid descriptor error...";
        }

    } // end 发现服务
}

// 特性发生变化的槽函数，处理接收数据：
void bluetooth_controller::slot_service_characteristic_changed(QLowEnergyCharacteristic c, QByteArray value)
{
    qDebug() << "slot_service_characteristic_changed : " << c.uuid().toString() << ", properties = " << c.properties();
    // if (QLowEnergyCharacteristic::Notify == c.properties())
    // {
    //     // 接收数据
    //     slot_service_recv_data(c, value);
    // }
}

void bluetooth_controller::slot_service_descriptor_written(const QLowEnergyDescriptor &descriptor, const QByteArray &value)
{
    // 当描述符descriptor被正确地写入值之后，BLE通讯就成功建立。
    qDebug() << "slot_service_descriptor_written...";
}

void bluetooth_controller::slot_service_character_written(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) // 发送成功
{
    QByteArray hex_array = value.toHex(':');
    qDebug() << "slot service character written success..." << hex_array.data();
    
    ble_notify_service_->readCharacteristic(notify_characteristic_);   // 接收数据
}

// 特征为read时收到ble回传的数据会触发该函数
void bluetooth_controller::slot_service_recv_data(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    QByteArray hex_array = value.toHex(':');
    qDebug() << "bluetooth recv , character = " << c.uuid().toString() << ", data_len = " << value.size() << ", data = " << hex_array.data();
}

// BLE服务发生错误的槽函数
void bluetooth_controller::slot_service_error(QLowEnergyService::ServiceError error) // 低功耗蓝牙服务产生错误
{
    QString err_msg;
    if (QLowEnergyService::NoError == error) // 没有发生错误。
        return;
    else if (QLowEnergyService::OperationError == error)
    {
        qDebug() << "QLowEnergyService::OperationError...";
    }
    else if (QLowEnergyService::CharacteristicReadError == error)
    {
        qDebug() << "QLowEnergyService::CharacteristicReadError...";
    }
    else if (QLowEnergyService::CharacteristicWriteError == error)
    {
        qDebug() << "QLowEnergyService::CharacteristicWriteError...";
    }
    else if (QLowEnergyService::DescriptorReadError == error)
    {
        qDebug() << "QLowEnergyService::DescriptorReadError...";
    }
    else if (QLowEnergyService::DescriptorWriteError == error)
    {
        qDebug() << "QLowEnergyService::DescriptorWriteError...";
    }
    else
    {
        qDebug() << "unkonw error...";
    }
}

void bluetooth_controller::write(const QByteArray &data)
{
    if(ble_write_service_ && write_characteristic_.isValid())
    {
        if(data.length() > CHUNK_SIZE)
        {
            int sentBytes = 0;
            while (sentBytes < data.length())
            {
                ble_write_service_->writeCharacteristic(write_characteristic_,data.mid(sentBytes, CHUNK_SIZE),write_mode_);
                sentBytes += CHUNK_SIZE;
            }
        }
        else
        {
             ble_write_service_->writeCharacteristic(write_characteristic_, data, write_mode_);
        }

    }
}

void bluetooth_controller::test_send()
{
    // 在这里写一条数据试试
        // 握手命令蓝牙
        //        5A 05 82 E2 D2
               uint8_t arr[] = {0x5A, 0x05, 0x82, 0xE2, 0xD2};
        // 取得设备记录条数
        //        5A 05 53 BE 12
        // uint8_t arr[] = {0x5A, 0x05, 0x53, 0xBE, 0x12};
        // 读取得指定记录数据命令
        //        5A 07 54 00 1E A1 BC
        //  uint8_t arr[] = {0x5A, 0x07, 0x54, 0x1E, 0xA1,0xBC};


        QByteArray byte;
        byte = QByteArray((char*)arr,sizeof (arr));
        write(byte);
}