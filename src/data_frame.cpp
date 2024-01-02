#include "data_frame.h"
#include <QDebug>

namespace medex 
{
    namespace hut
    {
        data_frame::data_frame(QObject *parent) : QObject(parent)
        {

        }

        bool data_frame::parse(const uint8_t* package)
        {
            // 检查长度
            if (sizeof (package) < 5) {
                return false;
            }

            // 提取字段
            header = package[0];
            package_length = package[1];
            instruction = package[2];
            data_length = package_length - 5;
            data = new unsigned char[data_length];
            memcpy(data, package + 3, data_length);
            if (instruction  == 0x52)
            {
                // 下位机返回应答的 CRC16 特殊，是高位在后，低位在前。（这跟星脉动态血压当初设计有关）
                checksum = package[package_length - 1] << 8 | package[package_length - 2];
            }
            else
            {
                checksum = package[package_length - 2] << 8 | package[package_length - 1];
            }

            // -- printf --
            // 原始包内容
            qDebug() << "package... ";
            for (uint16_t i = 0; i < package_length; i++)
            {
                qDebug() << "package["<< i <<"]:"<<QString::number(package[i], 16).toStdString().c_str();
            }

            // 包头
            qDebug() <<"header: "<< QString::number(header, 16).toStdString().c_str();
            // 包长
            qDebug() << "package_length: "<< package_length;
            // 数据长
            qDebug() << "data_length: "<< data_length;
            // 指令
            qDebug() <<"instruction: "<< QString::number(instruction, 16).toStdString().c_str();
            // 数据
            qDebug() << "data... ";
            for (uint16_t i = 0; i < data_length; i++)
            {
                qDebug() << "data["<<i<<"]:"<<QString::number(data[i], 16).toStdString().c_str();
            }
            // 校验和
            qDebug() <<"checksum: "<< QString::number(checksum, 16).toStdString().c_str();
            // -- printf --

            // 计算校验和
            crcdata = new unsigned char[package_length - 2];
            memcpy(crcdata, package, package_length - 2);
            uint16_t calc_checksum = crc16_mobus(crcdata, package_length - 2);
            qDebug() <<"calc_checksum: "<< QString::number(calc_checksum, 16).toStdString().c_str();
            // 比较校验和
            if (checksum != calc_checksum)
            {
                qDebug() <<"crc16 value not equal package error...";
                return false;
            }

            qDebug() <<"parse finish...";
            return true;
        }

        unsigned short data_frame::crc16_mobus(unsigned char *data, unsigned int len)
        {
            unsigned short CRCin = 0xffff;
            unsigned short CRCret = 0;
            for(unsigned int i = 0; i < len; i++)
            {
                CRCin = *data ^ CRCin;
                for(int j = 0; j < 8; j++)
                {
                    if(CRCin & 0x01)
                    {
                        CRCin = CRCin >> 1;
                        CRCin = CRCin ^ 0xa001;     //0xa001是由0x8005高低位转换所得
                    }
                    else
                    {
                        CRCin = CRCin >> 1;
                    }
                }
                data++;
            }
            CRCret = CRCin >> 8;
            CRCret = CRCret | (CRCin << 8);

            CRCin = CRCret >> 8;
            CRCin = CRCin | (CRCret << 8);
            return CRCin;
        }
    }// hut
}// medex

