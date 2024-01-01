#include "bluetooth_controller.h"
#include "instruction_set.h"

bluetooth_controller::bluetooth_controller(QObject *parent) : QObject(parent)
{
    ble_scaner_ = new bluetooth_scaner(this);
    ble_controller_ = nullptr;
    ble_write_service_ = nullptr;
    ble_notify_service_ = nullptr;
    write_mode_ = QLowEnergyService::WriteWithResponse;
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
    connect(ble_controller_, &QLowEnergyController::disconnected, this, &bluetooth_controller::slot_device_disconnected);           //设备断开链接

    // 发现服务
    connect(ble_controller_, &QLowEnergyController::serviceDiscovered, this, &bluetooth_controller::slot_service_discovered_one);   // 发现一个服务
    connect(ble_controller_, &QLowEnergyController::discoveryFinished, this, &bluetooth_controller::slot_service_discovery_finish); // 服务发现结束
}

void bluetooth_controller::bind_write_service_slot()
{
    // 监听服务状态变化
    connect(ble_write_service_, &QLowEnergyService::stateChanged, this, &bluetooth_controller::slot_write_service_state_changed);
    void (QLowEnergyService::*bleServiceErrorOccurred)(QLowEnergyService::ServiceError) = &QLowEnergyService::error;                    // 有重载
    connect(ble_write_service_, bleServiceErrorOccurred, this, &bluetooth_controller::slot_service_error);
    connect(ble_write_service_, &QLowEnergyService::characteristicWritten, this, &bluetooth_controller::slot_service_character_written);// 写数据成功返回
}

void bluetooth_controller::bind_notify_service_slot()
{
    // 监听服务状态变化
    connect(ble_notify_service_, &QLowEnergyService::stateChanged, this, &bluetooth_controller::slot_notify_service_state_changed);
    // 服务的characteristic变化,有数据传来
    // 备注：Notify的特征的数据回传触发的是QLowEnergyService::characteristicChanged，并不是QLowEnergyService::characteristicRead
    connect(ble_notify_service_, &QLowEnergyService::characteristicChanged, this, &bluetooth_controller::slot_service_characteristic_changed);  // Notify的特征的数据回传触发
    void (QLowEnergyService::*bleServiceErrorOccurred)(QLowEnergyService::ServiceError) = &QLowEnergyService::error;                            // 错误处理有重载
    connect(ble_notify_service_, bleServiceErrorOccurred, this, &bluetooth_controller::slot_service_error);
    connect(ble_notify_service_, &QLowEnergyService::descriptorWritten, this, &bluetooth_controller::slot_service_descriptor_written);          // 描述符成功被写
}

void bluetooth_controller::slot_device_connected()
{
    qDebug() << "device connected success and begin discover service...";
    discover_services();
}

void bluetooth_controller::slot_device_disconnected()
{
    qDebug() << "device disconnected...";
}

void bluetooth_controller::slot_device_connect_error(QLowEnergyController::Error error)
{
    qDebug() << "connect device error..." << error;
}

void bluetooth_controller::create_ble_controller()
{
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
        ble_controller_->connectToDevice();
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
    qDebug() << "found one service uuid:"<< service_uuid.toString() << " toUint:" << service_uuid.toUInt32();

    if (service_uuid == QBluetoothUuid(QString::fromStdString(WRITE_SERVER_UUID)))
    {
        qDebug() << "found write service uuid:" << service_uuid.toString();
        wirte_service_uuid_ = service_uuid;
    }

    if (service_uuid == QBluetoothUuid(QString::fromStdString(NOTIFY_SERVER_UUID)))
    {
        qDebug() << "found notify service uuid:" << service_uuid.toString();
        notify_service_uuid_ = service_uuid;
    }
}

// 服务搜索结束
void bluetooth_controller::slot_service_discovery_finish()
{
    qDebug() << "discover service finish...";
    // 创建读服务对象读取写服务详情
    QTimer::singleShot(1000, [=](){
         this->create_write_service_object();
    });

    // 创建通知服务对象读取通知服务详情
    QTimer::singleShot(1000, [=](){
        this->create_notify_service_object();
    });

    QTimer::singleShot(2000, [=](){
        this->test_send1();
    });

    QTimer::singleShot(3000, [=](){
        this->test_send2();
    });

    QTimer::singleShot(4000, [=](){
        this->test_send3();
    });

   
}

void bluetooth_controller::create_write_service_object()
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
        qDebug() << "create service object failed sevice_id is:"<< wirte_service_uuid_.toString();
    }
}

void bluetooth_controller::create_notify_service_object()
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
        qDebug() << "create service object failed sevice_id is:"<< notify_service_uuid_.toString();
    }
}

// 服务状态变化
void bluetooth_controller::slot_write_service_state_changed(QLowEnergyService::ServiceState state) // 服务状态改变
{
    if (QLowEnergyService::ServiceDiscovered == state) // 发现服务
    {
        qDebug() << "write service state is ServiceDiscovered...";
        QList<QLowEnergyCharacteristic> list = ble_write_service_->characteristics();
        for (int i = 0; i < list.count(); i++)
        {
            // 当前位置的bleCharacteritic
            QLowEnergyCharacteristic cur_character = list.at(i);
            // 如果当前characteristic有效
            if (cur_character.isValid())
            {
                qDebug() << "cur_character(" << i << ")is valid"
                    << "\nname:" << cur_character.name()
                    << "\nproperty:" << cur_character.properties()
                    << "\nuuid:" << cur_character.uuid().toString()
                    << "\nisvalid:" << cur_character.isValid()
                    << "\nvalue:" << cur_character.value().data()
                    << "\ndescriptors count:" << cur_character.descriptors().count();

                QList<QLowEnergyDescriptor> desc_list = cur_character.descriptors();
                for (int d = 0; d < desc_list.count(); d++)
                {
                    qDebug() << "descriptor[" << d << "]: name: " << desc_list.at(d).name()
                        << "\ntype: " << desc_list.at(d).type()
                        << "\nuuid: " << desc_list.at(d).uuid().toString()
                        << "\nvalue: " << desc_list.at(d).value();
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                {
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
                    read_characteristic_ = cur_character;
                    qDebug() << "this characteristic has Read property" << cur_character.uuid().toString();
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Notify)
                {
                    notify_characteristic_ = cur_character;
                    qDebug() << "this characteristic has Notify property" << cur_character.uuid().toString();
                }

                // 描述符定义特征如何由特定客户端配置
                QLowEnergyDescriptor descriptor = cur_character.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (descriptor.isValid())
                {
                    // 这里不做处理根据实际情况进行描述符写入
                }
            }
        } // end for list
    } // end 发现服务
}

// 服务状态变化
void bluetooth_controller::slot_notify_service_state_changed(QLowEnergyService::ServiceState state) // 服务状态改变
{
    if (QLowEnergyService::ServiceDiscovered == state) // 发现服务
    {
        qDebug() << "notify service state is ServiceDiscovered...";
        QList<QLowEnergyCharacteristic> list = ble_notify_service_->characteristics();
        qDebug() << "QLowEnergyCharacteristic list count:" << list.count();
        for (int i = 0; i < list.count(); i++)
        {
            // 当前位置的bleCharacteritic
            QLowEnergyCharacteristic cur_character = list.at(i);
            // 如果当前characteristic有效
            if (cur_character.isValid())
            {
                qDebug() << "cur_character("<< i <<") is valid"
                    << "\nname:" << cur_character.name()
                    << "\nproperty:" << cur_character.properties()
                    << "\nuuid:" << cur_character.uuid().toString()
                    << "\nisvalid:" << cur_character.isValid()
                    << "\nvalue:" << cur_character.value()
                    << "\ndescriptors count:" << cur_character.descriptors().count();

                QList<QLowEnergyDescriptor> desc_list = cur_character.descriptors();
                for (int d = 0; d < desc_list.count(); d++)
                {
                    qDebug() << "descriptor[" << d << "] name:" 
                        << desc_list.at(d).name()
                        << "\ntype:" << desc_list.at(d).type()
                        << "\nuuid:" << desc_list.at(d).uuid().toString()
                        << "\nvalue:" << desc_list.at(d).value();
                }

                if (cur_character.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                {
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
                    read_characteristic_ = cur_character;
                    qDebug() << "this characteristic has Read property" << cur_character.uuid().toString();
                }
                if (cur_character.properties() & QLowEnergyCharacteristic::Notify)
                {
                    notify_characteristic_ = cur_character;
                    qDebug() << "this characteristic has Notify property" << cur_character.uuid().toString();
                }

                // 描述符定义特征如何由特定客户端配置
                QLowEnergyDescriptor descriptor = cur_character.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (descriptor.isValid())
                {
                    if(cur_character.properties() & QLowEnergyCharacteristic::Notify)
                    {
                        // 0100 enable notification  0000 disable notification
                        ble_notify_service_->writeDescriptor(descriptor, QByteArray::fromHex("0100")); 
                    }
                }
            }
        } // end for list

    } // end 发现服务
}

// 特性发生变化的槽函数，处理接收数据：
void bluetooth_controller::slot_service_characteristic_changed(QLowEnergyCharacteristic c, QByteArray value)
{
    if (QLowEnergyCharacteristic::Notify == c.properties())
    {
        // 接收通知数据
        receive_notify_data(c, value);
    }
}

void bluetooth_controller::slot_service_descriptor_written(const QLowEnergyDescriptor &descriptor, const QByteArray &value)
{
    QByteArray hex_array = value.toHex(':');
    // 当描述符descriptor被正确地写入值之后，BLE通讯就成功建立。
    qDebug() <<"descriptor: "  << descriptor.uuid().toString()<<" value:" << hex_array.data()<< " descriptor written success...";
}

void bluetooth_controller::slot_service_character_written(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) // 发送成功
{
    QByteArray hex_array = value.toHex(':');
    qDebug() << "data written success..." << hex_array.data();
}

// 特征为read时收到ble回传的数据会触发该函数
void bluetooth_controller::receive_notify_data(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    QByteArray hex_array = value.toHex(':');
    qDebug() << "bluetooth recvive notify data:" << hex_array.data();
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

void bluetooth_controller::wait_for_write()
{
    QEventLoop loop;
    connect(ble_write_service_, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)), &loop, SLOT(quit()));
    loop.exec();
}

void bluetooth_controller::write(uint8_t* arr, uint16_t len)
{
    QByteArray data = QByteArray((char*)arr, len);
    if(ble_write_service_ && write_characteristic_.isValid())
    {
        if(data.length() > CHUNK_SIZE)
        {
            int sentBytes = 0;
            while (sentBytes < data.length())
            {
                ble_write_service_->writeCharacteristic(write_characteristic_,data.mid(sentBytes, CHUNK_SIZE),write_mode_);
                sentBytes += CHUNK_SIZE;
                if(write_mode_ == QLowEnergyService::WriteWithResponse)
                {
                    wait_for_write();
                    if(ble_write_service_->error() != QLowEnergyService::NoError)
                    {
                        qDebug() << "write data error...";
                         return;
                    }
                }
            }
        }
        else
        {
             ble_write_service_->writeCharacteristic(write_characteristic_, data, write_mode_);
        }
    }
}

void bluetooth_controller::test_send1()
{
    // 在这里写一条数据试试
    // 握手命令蓝牙
    // 5A 05 82 E2 D2
    uint8_t arr[] = {0x5A, 0x05, 0x82, 0xE2, 0xD2};
    QByteArray byte;
    byte = QByteArray((char*)arr,sizeof (arr));
    write(hand_shake_cmd, sizeof(hand_shake_cmd));
}

void bluetooth_controller::test_send2()
{
    // 取得设备记录条数
    // 5A 05 53 BE 12
    uint8_t arr[] = {0x5A, 0x05, 0x53, 0xBE, 0x12};
    QByteArray byte;
    byte = QByteArray((char*)arr,sizeof (arr));
   write(get_record_count_cmd, sizeof(get_record_count_cmd));
}

void bluetooth_controller::test_send3()
{
    // 读取得指定记录数据命令
    //  5A 07 54 00 1E A1 BC
    uint8_t arr[] = {0x5A, 0x07, 0x54, 0x00, 0x1E, 0xA1,0xBC};
    QByteArray byte;
    byte = QByteArray((char*)arr,sizeof (arr));
    write(get_specified_record_cmd, sizeof(get_specified_record_cmd));
}