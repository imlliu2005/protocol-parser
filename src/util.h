/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2024-1-3 
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2024-1-3
 * @FilePath: 
 */

#include <cstdint>
#include <string>
#include <iostream>
#include <cmath>
#include <QByteArray>

#ifndef DATA_UTIL_H
#define DATA_UTIL_H

namespace medex
{
    namespace hut
    {
        uint8_t byte_low_4bit(uint8_t byte);                                    // 取一个字节低4位
        uint8_t byte_high_4bit(uint8_t byte);                                   // 取一个字节高4位
        uint16_t crc16_mobus(uint8_t *data, uint16_t len);                      // 计算crc16mobus校验和
        uint16_t merge_byte_to_uint16(uint8_t b1, uint8_t b2);                  // 合并两个字节为一个16位的数
        QByteArray uint8array_to_qbytearray(uint8_t* arr, uint16_t arr_len);    // 转换uint8类型数组为QByteArray
        void generate_crc16_cmd(uint8_t *arr);

    }// hut
}// medex
#endif