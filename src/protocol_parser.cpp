/*
 * @Description:
 * @Author: liuning
 * @LastEditors: liuning
 * @Date: 2023-12-29
 * @Copyright: 北京麦迪克斯科技有限公司
 * @LastEditTime: 2024-1-2 
 * @FilePath: 
 */

#include "protocol_parser.h"
#include "package_instructions.h"
#include "util.h"
#include <QDebug>
#include "bluetooth_controller.h"

namespace medex 
{
    namespace hut
    {
        protocol_parser::protocol_parser(bluetooth_controller* bc)
        {
            df_ = new data_frame();
            bc_ = bc;
        }

        protocol_parser::~protocol_parser()
        {
            if(df_)
            {
                delete df_;
                df_ = nullptr;
            }
            if(bc_) 
            {
                delete bc_;
                bc_ = nullptr;
            }
        }

        record_data protocol_parser::parse_record_data(uint8_t* data)   // 返回血压计指定记录的详细数据
        {
            record_data rd;          
            rd.sys = merge_byte_to_uint16(data[0], data[1]);
            qDebug() << "sys:" << rd.sys;
            rd.dia = merge_byte_to_uint16(data[2], data[3]);
            qDebug() << "dia:" << rd.dia;
            rd.rate = merge_byte_to_uint16(data[4], data[5]);
            qDebug() << "rate:" << rd.rate;
            rd.year = data[6];
            rd.mounth = data[7];
            rd.date = data[8];
            rd.hour = data[9];
            rd.min = data[10];
            qDebug() << "date: " << rd.year<<":"<<rd.mounth<<":"<<rd.date<<":"<<rd.hour<<":"<<rd.min;
            rd.ec = data[11];
            qDebug() << "ec:" << rd.ec;
            rd.auto_byte = data[12];
            rd.auto_high = byte_high_4bit(data[12]);
            qDebug() << "auto_high:" << rd.auto_high;
            if (rd.auto_high == 0) 
            {
                qDebug() << "measurement mode: auto measure" ;
            }
            if (rd.auto_high == 1) 
            {
                qDebug() << "measurement mode: measure by hand" ;
            }
            if (rd.auto_high == 2) 
            {
                qDebug() << "measurement mode: auto measure again" ;
            }
            
            rd.auto_low = byte_low_4bit(data[12]);
            qDebug() << "auto_low:" << rd.auto_low;
            if (rd.auto_low == 0) 
            {
                qDebug() << "Position: stand" ;
            }
            if (rd.auto_low == 1) 
            {
                qDebug() << "Position: lie low" ;
            }
            if (rd.auto_low == 2) 
            {
                qDebug() << "Position: movement" ;
            }

            return rd;
        }

        void protocol_parser::parser_receive_data(QByteArray value)
        {
            const uint8_t* arr = reinterpret_cast<const uint8_t*>(value.data());
            if (df_->parse(arr)) 
            {
                uint16_t record_count;         // 数据记录条数
                switch (df_->instruction_)
                {
                    case 0x44: // 设置 5 分钟后开始测量
                        if (df_->data_[0] == 0x01)
                        {
                            qDebug() << "instituation is 0x44 enable measured every 5 minutes...";
                        }
                        else if (df_->data_[0] == 0x00)
                        {
                            qDebug() << "instituation is 0x44 disabled measured every 5 minutes...";
                        }
                    break; 

                    case 0x45: // 禁止使用按键
                        if (df_->data_[0] == 0x01)
                        {
                            qDebug() << "instituation is 0x45 set device enable push key...";
                        }
                        else if (df_->data_[0] == 0x00)
                        {
                            qDebug() << "instituation is 0x45 set device disabale push key...";
                        }
                        break;

                        case 0x46: // 设置固定/标准时间测量
                        if (df_->data_[0] == 0x01)
                        {
                            qDebug() << "instituation is 0x46 set device fix time ...";
                        }
                        else if (df_->data_[0] == 0x00)
                        {
                            qDebug() << "instituation is 0x46 set device stranderd time...";
                        }
                        break;

                    case 0x53: // 取得设备记录条数
                        record_count = merge_byte_to_uint16(df_->data_[0], df_->data_[1]);
                        qDebug() << "instituation is 0x53 get reord counts: "<< record_count << "...";
                        for (int i = 0; i < record_count; i++) 
                        {   
                            uint8_t * cmd = get_specified_record_cmd(i);
                            bc_->send_instruction(cmd);
                        }
                    break;

                    case 0x48: // 设置白天开始时间
                        qDebug() << "instituation is 0x48 set day begin time...";
                        break;
                    
                    case 0x49: // 设置白天时间间隔
                        qDebug() << "instituation is 0x49 set day interval time...";
                        break;

                    case 0x4A: // 设置夜间开始时间
                        qDebug() << "instituation is 0x4A set night begin time...";
                        break;
                    
                    case 0x4B: // 设置夜间时间间隔
                        qDebug() << "instituation is 0x4B set night interval time...";
                        break;

                    case 0x4C: // 设特殊1开始时间
                        qDebug() << "instituation is 0x4C set special 1 begin time...";
                        break;

                    case 0x4D: // 设特殊1结束时间
                        qDebug() << "instituation is 0x4D set special 1 end time...";
                        break;

                    case 0x4E: // 设特殊1时间间隔
                        qDebug() << "instituation is 0x4E set special 1 interval time...";
                        break;

                    case 0x54: // 读取得指定记录数据命令
                        qDebug() << "instituation is 0x54 get special record...";
                        parse_record_data(df_->data_);
                        break;

                    case 0x57: // 设备内记录清 0
                        qDebug() << "instituation is 0x57 clear records...";
                    break;

                    case 0x58:
                        if (df_->data_[0] == 0x01)
                        {
                            qDebug() << "instituation is 0x58 set special time measure enabled cmd...";
                        }
                        else if (df_->data_[0] == 0x00)
                        {
                            qDebug() << "instituation is 0x58 set special time measure disabled cmd...";
                        }
                    break;

                default:
                    break;
                }
            }
            else
            {
                qDebug() <<"data parser failed...";
            }
        }
    }// hut
}// medex
