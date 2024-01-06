/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2023-12-27 
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2023-12-27 
 * @FilePath: 
 */

#ifndef DATA_FRAME_H
#define DATA_FRAME_H

#include <cstdint>

namespace medex 
{
    namespace hut
    {
        class data_frame
        {
        public:
            data_frame();
            ~data_frame();
            bool parse(const uint8_t *package);   // 解析函数

        public:
            uint8_t header_;         // 帧头（Frame Header）
            uint8_t package_length_; // 整个包长度
            uint8_t instruction_;    // 指令（Instruction）
            uint8_t* data_;          // 数据（Data）
            uint16_t checksum_;      // 校验和（Checksum）
        };
    }// hut
}// medex

#endif // DATA_FRAME_H

