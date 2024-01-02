#ifndef DATA_FRAME_H
#define DATA_FRAME_H

#include <QObject>

namespace medex 
{
    namespace hut
    {
        
        class data_frame : public QObject
        {
            Q_OBJECT
        public:
            explicit data_frame(QObject *parent = nullptr);
            bool parse(const uint8_t *package);   // 解析函数

        public:
            uint8_t header;         // 帧头（Frame Header）
            uint8_t data_length;    // 数据长度（Data Length）是整个包的长度(package_length) - 数据头 - 指令 - 包长 - 2个校验码
            uint8_t package_length; // 整个包长度
            uint8_t instruction;    // 指令（Instruction）
            uint8_t* data;          // 数据（Data）
            uint16_t checksum;      // 校验和（Checksum）
            uint8_t* crcdata;       // 数据（Data）

        private:
            unsigned short crc16_mobus(unsigned char *data, unsigned int len);

        signals:

        };
    }// hut
}// medex

#endif // DATA_FRAME_H

