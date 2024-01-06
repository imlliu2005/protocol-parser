/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2024-1-3 
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2024-1-3
 * @FilePath: 
 */
#include "util.h"
#include <QDebug>

namespace medex
{
    namespace hut
    {
        uint8_t byte_low_4bit(uint8_t byte)
        {
            return byte & 0x0f;
        }

        uint8_t byte_high_4bit(uint8_t byte)
        {
             return (byte & 0xf0) >> 4;
        }

        uint16_t merge_byte_to_uint16(uint8_t b1, uint8_t b2)
        {
            return (uint16_t)((b1 & 0xFF) << 8 | b2 & 0xFF);
        }

        QByteArray uint8array_to_qbytearray(uint8_t* arr, uint16_t arr_len)
        {
            return QByteArray((char*)arr, arr_len);
        }

        void generate_crc16_cmd(uint8_t *arr)
        {
            uint16_t len = arr[1];
            uint16_t checksum = crc16_mobus(arr, len - 2);
            uint8_t hight8, low8;
            hight8 = (checksum >> 8) & 0xff; 
            low8 = checksum & 0xff;
            arr[len -2] = hight8;
            arr[len -1] = low8;
            // for (uint16_t i = 0; i < len; i++)
            // {
            //     qDebug() << "cmd_with_crc16["<< i <<"]:"<<QString::number(arr[i], 16).toStdString().c_str();
            // }
        }

        uint16_t crc16_mobus(uint8_t *data, uint16_t len)
        {
            uint16_t crc = 0xffff;
            for(uint16_t i = 0; i < len; i++)
            {
                crc = *data ^ crc;
                for(int j = 0; j < 8; j++)
                {
                    if(crc & 0x01)
                    {
                        crc = crc >> 1;
                        crc = crc ^ 0xa001; // 0xa001是由0x8005高低位转换所得
                    }
                    else
                    {
                        crc = crc >> 1;
                    }
                }
                data++;
            }
            return crc;
        }

    }//hut
}//medex
