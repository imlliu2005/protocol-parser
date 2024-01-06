/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2023-12-27 
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2023-12-27 
 * @FilePath: 
 */

#include "data_frame.h"
#include <QDebug>
#include "util.h"

namespace medex 
{
    namespace hut
    {
        data_frame::data_frame()
        {
            data_ = nullptr;
        }

        data_frame::~data_frame()
        {
             if (data_) 
            {
                delete[] data_;
                data_ = nullptr;
            }
        }

        bool data_frame::parse(const uint8_t* package)
        {
            // 检查长度
            if (sizeof (package) < 5) {
                qDebug() << "package length < 5 package error...";
                return false;
            }

            // 提取字段
            header_ = package[0];
            package_length_ = package[1];
            instruction_ = package[2];
            if (data_) 
            {
                delete[] data_;
                data_ = nullptr;
            }

            uint16_t data_len = package_length_ - 5;
            data_ = new uint8_t[data_len];

            memcpy(data_, package + 3, data_len);
            if (instruction_ == 0x52)
            {
                // 下位机返回应答的 CRC16 特殊，是高位在后，低位在前。（这跟星脉动态血压当初设计有关）
                checksum_ = package[package_length_ - 1] << 8 | package[package_length_ - 2];
            }
            else
            {
                checksum_ = package[package_length_ - 2] << 8 | package[package_length_ - 1];
            }

            // 计算校验和
            uint8_t* crc_data = new unsigned char[package_length_ - 2];
            memcpy(crc_data, package, package_length_ - 2);
            uint16_t calc_checksum = crc16_mobus(crc_data, package_length_ - 2);
            if (crc_data)
            {
                delete[] crc_data;
                crc_data = nullptr;
            }
            // 比较校验和
            if (checksum_ != calc_checksum)
            {
                qDebug() <<"crc16 checksum value not equal parse package error...";                
                return false;
            }

            return true;
        }
    }// hut
}// medex

