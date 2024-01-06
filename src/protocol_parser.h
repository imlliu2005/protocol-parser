/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2023-12-29
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2024-1-2 
 * @FilePath: 
 */

#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <QObject>
#include "data_frame.h"

namespace medex 
{
    namespace hut
    {
        class bluetooth_controller;

        typedef struct record_data // 血压记录的详细记录
        {
            uint16_t sys;       // SYS:     2 字节 116: 0x00 0x74
            uint16_t dia;       // DIA:     2 字节 76: 0x00 0x4C
            uint16_t rate;      // RATE:    2 字节 67: 0x00 0x43
            uint8_t year;       // YEAR :   1 字节 12:0x0C
            uint8_t mounth;     // MONTH:   1 字节 6:0x06
            uint8_t date;       // DATE:    1 字节 23:0x17
            uint8_t hour;       // HOUR:    1 字节 6:0x06
            uint8_t min;        // MIN:     1 字节 24:0x18
            uint8_t ec;         // EC:      1 字节 0:0x00
            uint8_t auto_byte;  // AUTO:    1 字节 0:0x00 
                                // BIT0—BIT2 测量方式 0000:自动测量 0001:手动测量 0010:自动重测
                                // BIT4—BIT7 体位信息 0000: 平躺    0001:站立     0010:轻微运动 
            uint8_t auto_high;  // auto_byte 字节高4位
            uint8_t auto_low;   // auto_byte 字节低4位
        } record_data;

        class protocol_parser : public QObject
        {
            Q_OBJECT
        public:
            protocol_parser(bluetooth_controller *bc);
            ~protocol_parser();
            // receive data slot
            void parser_receive_data(QByteArray value);

            signals:

        private:
            record_data parse_record_data(uint8_t* data);

        private:
            bluetooth_controller *bc_; 
            data_frame* df_;                // 协议包解析类对象     

        };
    }// hut
}// medex


#endif // PROTOCOL_PARSER_H
