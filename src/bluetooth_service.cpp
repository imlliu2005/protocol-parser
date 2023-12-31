#include "bluetooth_service.h"

bluetooth_service::bluetooth_service()
{
 
}

bluetooth_service::~bluetooth_service()
{

}

// 服务状态变化
void bluetooth_service::slot_service_state_changed(QLowEnergyService::ServiceState state) // 服务状态改变
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
                QLowEnergyDescriptor descriptor = cur_character.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                qDebug()<<"descriptor name : ... "<< descriptor.name();
                if (descriptor.isValid())
                {
                    qDebug() << "--- this descriptor (" << i << ") is valid";
                    ble_write_service_->writeDescriptor(descriptor, QByteArray::fromHex("0100")); // 00FF, FF01, FF02 TODO 这里应该写啥？
                    found_valid_descriptor = true;
                }
            }
        } // end for list

        if (!found_valid_descriptor)
        {
            qDebug() <<"found_valid_descriptor error...";
        }

        // 在这里写一条数据试试
        // 握手命令蓝牙
        //        5A 05 82 E2 D2
        //        uint8_t arr[] = {0x5A, 0x05, 0x82, 0xE2, 0xD2};
        // 取得设备记录条数
        //        5A 05 53 BE 12
        uint8_t arr[] = {0x5A, 0x05, 0x53, 0xBE, 0x12};
        // 读取得指定记录数据命令
        //        5A 07 54 00 1E A1 BC
        //                uint8_t arr[] = {0x5A, 0x07, 0x54, 0x1E, 0xA1,0xBC};
        qDebug()<<"write .... ";


        QByteArray byte;
        byte = QByteArray((char*)arr,sizeof (arr));
        // write(byte);
    } // end 发现服务
}

// 特性发生变化的槽函数，处理接收数据：
void bluetooth_service::slot_service_characteristic_changed(QLowEnergyCharacteristic c, QByteArray value)
{
    qDebug() << "-----slot_service_characteristic_changed : " << c.uuid().toString() << ", properties = " << c.properties();
    // LOG_DEBUG << "value = " << value.toHex(' ').data() << std::endl;
    if (QLowEnergyCharacteristic::Notify == c.properties())
    {
        // 接收数据
        slot_service_recv_data(c, value);
    }
}

void bluetooth_service::slot_service_descriptor_written(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue)
{
    // 当描述符descriptor被正确地写入值之后，BLE通讯就成功建立。
    qDebug() << "slot_service_descriptor_written";
}

void bluetooth_service::slot_service_character_written(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) // 发送成功
{
    qDebug() << "slot_service_character_written success..." << newValue;
    ble_write_service_->readCharacteristic(notify_characteristic_);             // 接收数据
}

// 特征为read时收到ble回传的数据会触发该函数
void bluetooth_service::slot_service_recv_data(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    qDebug()<< "hello ning...";
    QByteArray hex_array = value.toHex(':');
    qDebug() << "bluetooth recv , character = " << c.uuid().toString() << ", data_len = " << value.size() << ", data = " << hex_array.data();
}

// BLE服务发生错误的槽函数
void bluetooth_service::slot_service_error(QLowEnergyService::ServiceError error) // 低功耗蓝牙服务产生错误
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
