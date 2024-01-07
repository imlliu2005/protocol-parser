/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2023-12-28
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2023-12-28
 * @FilePath: 
 */

#ifndef DATA_PACKAGE_INSTRUCTIONS_H
#define DATA_PACKAGE_INSTRUCTIONS_H

#include <cstdint>
#include <memory>
#include "util.h"

namespace medex 
{
    namespace hut
    {
        // 1.设置用户名 0x5A 0x08 0x41 0x57 0x53 0x44 crc16
        uint8_t* set_user_name_cmd();
        // 2.设置用户ID 0x5A 0x0B 0x42 0x31 0x31 0x32 0x35 0x66 0x66 crc16
        uint8_t* set_user_id_cmd();
        // 3.设置最大压力值280  0x5A 0x07 0x43 0x01 0x18 crc16
        uint8_t* set_maximum_pressure_cmd(int number);
        // 4.设置5分钟后开始测量 0x5A 0x06 0x44 0x01 crc16  主机发送设置5分钟后是否启动测量 0x01:启用 0x00:禁用
        uint8_t* set_auto_5min_measure_enabled_cmd();
        uint8_t* set_auto_5min_measure_disabled_cmd();
        // 6.是否屏幕显示 0x01 启用 0x00:禁用 0x5A 0x06 0x46 0x01 crc16
        uint8_t* set_screen_show_enabled_cmd();
        uint8_t* set_screen_show_disabled_cmd();
        // 5.是否使用按键 0x01 启用 0x00:禁用 0x5A 0x06 0x45 0x00 crc16
        uint8_t* set_push_key_enabled_cmd();
        uint8_t* set_push_key_disabled_cmd();

        // 7.设置固定/标准时间测量模式 0x01:固定 0x00:标准  0x5A 0x06 0x46 0x01 crc16
        uint8_t* set_fixed_time_measure_cmd();
        uint8_t* set_standard_time_measure_cmd();

        // 8.设置白天开始时间 比如设置白天开始时间:8点30分  0x5A 0x07 0x48 0x08 0x1E crc16
        uint8_t* set_day_begin_time_cmd(int hour, int minute);

        // 9.设置白天间隔时间 白天间隔时间:120分钟 0x5A 0x06 0x49 0x78 crc16
        uint8_t* set_day_interval_time_cmd(int interval);

        // 10.设置夜间开始时间 夜间开始时间:19点00分 0x5A 0x07 0x4A 0x13 0x00 crc16
        uint8_t* set_night_begin_time_cmd(int hour, int minute);
        // 11.设置夜间间隔时间 夜间间隔时间:60分钟 0x5A 0x06 0x4B 0x3C crc16
        uint8_t* set_night_interval_time_cmd(int interval);

        // 12.设置特殊 1 开始时间 主机发送设置特殊 1 开始时间  特殊 1 开始时间: 6 点 00 分 0x5A 0x07 0x4C 0x06 0x00 crc16
        uint8_t* set_special_one_begin_time_cmd(int hour, int minute);
        
        // 13.设置特殊 1 结束时间 主机发送设置特殊 1 结束时间 特殊 1 结束时间: 11 点 30 分 0x5A 0x07 0x4D 0x0B 0x1E crc16
        uint8_t* set_special_one_end_time_cmd(int hour, int minute);

        // 14.设置特殊 1 间隔时间 主机发送设置特殊 1 间隔时间 特殊 1 间隔时间: 25 分钟 0x5A 0x06 0x4E 0x19 crc16
        uint8_t* set_special_one_interval_time_cmd(int interval);

        // 15.设置机器时间 主机发送设置设备 RTC 时间 设置设备的时间为: 2013 年 9 月 20 号 11 点 23 分  
        // 0x5A 0x0A 0x4F 0x0D 0x09 0x14 0x0B 0x17 crc16
        // 设置好时间后，下位机将按照这个时间进行计时及测量时间计算；
        uint8_t* set_rtc_time_cmd(int year, int mounth, int date, int hour, int minute);

        // 18.握手命令 0x5A 0x05 0x82 crc16
        uint8_t* hand_shake_cmd();

        // 19.取得设备记录条数 0x5A 0x05 0x53 crc16
        uint8_t* get_record_count_cmd();

        // 20.读取得指定记录数据命令 0x5A 0x07 0x54 0x00 0x01 crc16
        uint8_t* get_specified_record_cmd(int number);

        // 21.取得设备内的用户名 0x5A 0x05 0x55 crc16
        uint8_t* get_user_name_cmd();

        //22.取得设备内的用户ID 0x5A 0x05 0x56 crc16
        uint8_t* get_user_id_cmd();

        // 23.设备内记录清零 0x5A 0x05 0x57 crc16
        uint8_t* clean_device_records_cmd();

        // 24.特别时间使用标志  特别时间是否启用: 0x00—不启用 0x01: 启用 
        // 主机发送启用特殊时间段自动测量 0x5A 0x06 0x58 0x01 crc16
        uint8_t* set_special_time_measure_enabled_cmd();
        uint8_t* set_special_time_measure_disabled_cmd();

        // 25.设置 SYS 报警上限阀值 0x5A 0x07 0x59 0x00 0x64 crc16
        uint8_t* set_sys_alarm_up_limit_cmd(int number);

        // 26.设置 DIA 报警上限阀值 0x5A 0x07 0x5A 0x00 0x64 crc16
        uint8_t* set_dia_alarm_up_limit_cmd(int number);

        // 27.设置允许/禁止白天报警 0x5B 0x06 0x5B 0x01 crc16 ---0x00 禁止报警，0x01 启用报警
        uint8_t* set_day_alarm_enabled_cmd();
        uint8_t* set_day_alarm_disabled_cmd();

        // 28.设置允许/禁止夜间报警 0x5A 0x06 0x5C 0x01 crc16 ---0x00 禁止报警，0x01 启用报警
        uint8_t* set_night_alarm_enabled_cmd();
        uint8_t* set_night_alarm_disabled_cmd();

        // 29.设置特殊 2 开始时间 主机发送设置特殊 2 开始时间  特殊 2 开始时间: 6 点 00 分 0x5A 0x07 0x5D 0x06 0x00 crc16
        uint8_t* set_special_two_begin_time_cmd(int hour, int minute);
        
        // 30.设置特殊 2 结束时间 主机发送设置特殊 2 结束时间 特殊 2 结束时间: 11 点 30 分 0x5A 0x07 0x5E 0x0B 0x1E crc16
        uint8_t* set_special_two_end_time_cmd(int hour, int minute);

        // 31.设置特殊 2 间隔时间 主机发送设置特殊 2 间隔时间 特殊 2 间隔时间: 25 分钟 0x5A 0x06 0x5F 0x19 crc16
        uint8_t* set_special_two_interval_time_cmd(int interval);

        // 32.设置特殊 3 开始时间 主机发送设置特殊 3 开始时间  特殊 3 开始时间: 6 点 00 分 0x5A 0x07 0x60 0x06 0x00 crc16
        uint8_t* set_special_three_begin_time_cmd(int hour, int minute);
        
        // 33.设置特殊 3 结束时间 主机发送设置特殊 3 结束时间 特殊 3 结束时间: 11 点 30 分 0x5A 0x07 0x61 0x0B 0x1E crc16
        uint8_t* set_special_three_end_time_cmd(int hour, int minute);

        // 34.设置特殊 3 间隔时间 主机发送设置特殊 3 间隔时间 特殊 3 间隔时间: 25 分钟 0x5A 0x06 0x62 0x19 crc16
        uint8_t* set_special_three_interval_time_cmd(int interval);
 
        // 35.设置 SYS 报警下限阀值 0x5A 0x07 0x65 0x00 0x64 crc16 注：如果启用报警则当测量结束后检测到收缩压低于 100mmHg 则会报警提示；
        uint8_t* set_sys_alarm_low_limit_cmd(int number);

        // 36.设置 DIA 报警下限阀值 0x5A 0x07 0x66 0x00 0x64 crc16
        uint8_t* set_dia_alarm_low_limit_cmd(int number);

    }// hut
}// medex

#endif