/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2023-12-28
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2023-12-29
 * @FilePath: 
 */

#include "bluetooth_controller.h"
#include "util.h"
#include "package_instructions.h"

namespace medex 
{
    namespace hut
    {        
        bluetooth_controller::bluetooth_controller(QObject *parent) : QObject(parent)
        {
            ble_scaner_ = new bluetooth_scaner(this);
            parser_ = new protocol_parser(this);
            ble_controller_ = nullptr;
            ble_write_service_ = nullptr;
            ble_notify_service_ = nullptr;
            write_mode_ = QLowEnergyService::WriteWithResponse;
        }

        bluetooth_controller::~bluetooth_controller()
        {
            if(ble_controller_) {
                delete ble_controller_;
                ble_controller_ = nullptr;
            }
            if (ble_write_service_) 
            {
                delete ble_write_service_;
                ble_write_service_ = nullptr;   
            }
            if (ble_notify_service_)
            {
                delete ble_notify_service_;
                ble_notify_service_ = nullptr;
            }
            if (ble_scaner_)
            {
                delete ble_scaner_;
                ble_scaner_ = nullptr;
            }
            if (parser_)
            {
                delete parser_;
                parser_ = nullptr;
            }
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
            QTimer::singleShot(1000, [=](){
                this->discover_services();
            });
        }

        void bluetooth_controller::slot_device_disconnected()
        {
            qDebug() << "device disconnected...";
        }

        void bluetooth_controller::slot_device_connect_error(QLowEnergyController::Error error)
        {
            qDebug() << "connect device error..." << error;
        }

        void bluetooth_controller::discover_devices()
        {
            connect(ble_scaner_, &bluetooth_scaner::signal_device_found,this, &bluetooth_controller::slot_device_found);
            ble_scaner_->scan_bluetooth();
        }

        void bluetooth_controller::slot_device_found()
        {
            qDebug() << "begin connect device...";
            connect_device();
        }

        void bluetooth_controller::connect_device()
        {
            ble_controller_ = QLowEnergyController::createCentral(ble_scaner_->get_device_info()); // central相当于是主机
            if (ble_controller_) 
            {
                bind_device_slot();
                ble_controller_->connectToDevice();
            }
            else
            {
                qDebug() << "create controller failed...";
            }
        }

        void bluetooth_controller::discover_services()
        {
            ble_controller_->discoverServices(); // 开始搜索服务
        }

        // 发现一个服务
        void bluetooth_controller::slot_service_discovered_one(QBluetoothUuid service_uuid)
        {
            // qDebug() << "found one service uuid:"<< service_uuid.toString() << " toUint:" << service_uuid.toUInt32();

            if (service_uuid == QBluetoothUuid(QString::fromStdString(WRITE_SERVER_UUID)))
            {
                // qDebug() << "found write service uuid:" << service_uuid.toString();
                wirte_service_uuid_ = service_uuid;
            }

            if (service_uuid == QBluetoothUuid(QString::fromStdString(NOTIFY_SERVER_UUID)))
            {
                // qDebug() << "found notify service uuid:" << service_uuid.toString();
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
                QList<QLowEnergyCharacteristic> list = ble_write_service_->characteristics();
                for (int i = 0; i < list.count(); i++)
                {
                    // 当前位置的bleCharacteritic
                    QLowEnergyCharacteristic cur_character = list.at(i);
                    // 如果当前characteristic有效
                    if (cur_character.isValid())
                    {

                        if (cur_character.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                        {}
                        if (cur_character.properties() & QLowEnergyCharacteristic::Write)
                        {
                            write_characteristic_ = cur_character;
                            write_mode_ = QLowEnergyService::WriteWithResponse;
                            // qDebug() << "characteristic has Write property" << cur_character.uuid().toString();

                            // 在这里发送测试数据
                            QTimer::singleShot(1000, [=](){
                                this->test();
                            });
                        }
                        if (cur_character.properties() & QLowEnergyCharacteristic::Read)
                        {
                            read_characteristic_ = cur_character;
                            // qDebug() << "characteristic has Read property" << cur_character.uuid().toString();
                        }
                        if (cur_character.properties() & QLowEnergyCharacteristic::Notify)
                        {
                            notify_characteristic_ = cur_character;
                            // qDebug() << "characteristic has Notify property" << cur_character.uuid().toString();
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
                QList<QLowEnergyCharacteristic> list = ble_notify_service_->characteristics();
                for (int i = 0; i < list.count(); i++)
                {
                    // 当前位置的bleCharacteritic
                    QLowEnergyCharacteristic cur_character = list.at(i);
                    // 如果当前characteristic有效
                    if (cur_character.isValid())
                    {
                        if (cur_character.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                        {
                            // qDebug() << "characteristic has WriteNoResponse property" << cur_character.uuid().toString();
                        }
                        if (cur_character.properties() & QLowEnergyCharacteristic::Write)
                        {
                            write_characteristic_ = cur_character;
                            write_mode_ = QLowEnergyService::WriteWithResponse;
                            // qDebug() << "characteristic has Write property" << cur_character.uuid().toString();
                        }
                        if (cur_character.properties() & QLowEnergyCharacteristic::Read)
                        {
                            read_characteristic_ = cur_character;
                            // qDebug() << "characteristic has Read property" << cur_character.uuid().toString();
                        }
                        if (cur_character.properties() & QLowEnergyCharacteristic::Notify)
                        {
                            notify_characteristic_ = cur_character;
                            // qDebug() << "characteristic has Notify property" << cur_character.uuid().toString();
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
            // QByteArray hex_array = value.toHex(':');
            // qDebug() << "bluetooth recvive notify data:" << hex_array.data();
            if (QLowEnergyCharacteristic::Notify == c.properties())
            {
                // 接收通知数据
                parser_->parser_receive_data(value);
            }
        }

        void bluetooth_controller::slot_service_descriptor_written(const QLowEnergyDescriptor &descriptor, const QByteArray &value)
        {
            QByteArray hex_array = value.toHex(':');
            // 当描述符descriptor被正确地写入值之后，BLE通讯就成功建立。
            // qDebug() <<"descriptor: "  << descriptor.uuid().toString()<<" value:" << hex_array.data()<< " descriptor written success...";
        }

        void bluetooth_controller::slot_service_character_written(const QLowEnergyCharacteristic &characteristic, const QByteArray &value) // 发送成功
        {
            // QByteArray hex_array = value.toHex(':');
            // qDebug() << "send instituation  success data:" << hex_array.data();
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

        void bluetooth_controller::send_instruction(uint8_t *arr)
        {
            QByteArray data = uint8array_to_qbytearray(arr,arr[1]);
            if(ble_write_service_ && write_characteristic_.isValid())
            {
                if(data.length() > CHUNK_SIZE)
                {
                    int sent_bytes = 0;
                    while (sent_bytes < data.length())
                    {
                        ble_write_service_->writeCharacteristic(write_characteristic_,data.mid(sent_bytes, CHUNK_SIZE),write_mode_);
                        sent_bytes += CHUNK_SIZE;
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

        void bluetooth_controller::test() 
        {
            // 获取数据
            // QTimer::singleShot(1000, [=](){
            //     send_instruction(get_record_count_cmd());
            // });

            // 数据清空
            // QTimer::singleShot(1000, [=](){
            //     std::cout << "clean_device_records_cmd" << std::endl;
            //     send_instruction(clean_device_records_cmd());
            // });

            // 禁止按键
            // QTimer::singleShot(1000, [=](){
            //     send_instruction(set_push_key_disabled_cmd());
            // });
            
            // 启用按键
            // QTimer::singleShot(1000, [=](){
            //     send_instruction(set_push_key_enabled_cmd());
            // });


            // 设置5分钟后开始测量 
            // QTimer::singleShot(1000, [=](){
            //     send_instruction(set_auto_5min_measure_enabled_cmd());
            // });

            // 设置标准测量方式/固定测量方式 
            // QTimer::singleShot(1000, [=](){
            //     send_instruction(set_fixed_time_measure_cmd());
            // });
            // 设置白天启动时间 9:30
            // QTimer::singleShot(2000, [=](){
            //     send_instruction(set_day_begin_time_cmd(16,00));
            // });
            // 设置白天时间间隔 10 
            // QTimer::singleShot(3000, [=](){
            //     send_instruction(set_day_interval_time_cmd(30));
            // });

            // 设置夜间启动时间 19:00
            QTimer::singleShot(1000, [=](){
                send_instruction(set_night_begin_time_cmd(20,00));
            });
            // 设置夜间时间间隔 30 
            QTimer::singleShot(2000, [=](){
                send_instruction(set_night_interval_time_cmd(10));
            });


            // 设置特别时间使用标志
            // QTimer::singleShot(1000, [=](){
            //     send_instruction(set_special_time_measure_enabled_cmd();
            // });
            // 设置特殊1开始时间
            // QTimer::singleShot(2000, [=](){
            //     send_instruction(set_special_one_begin_time_cmd(11, 30));
            // });
            // 设置特殊1结束时间
            // QTimer::singleShot(3000, [=](){
            //     send_instruction(set_special_one_end_time_cmd(12, 30));
            // });
            // 设置特殊1间隔时间
            // QTimer::singleShot(4000, [=](){
            //     send_instruction(set_special_one_interval_time_cmd(20));
            // });
        }
    } // hut
}// medex
