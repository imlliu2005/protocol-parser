#include "protocol_parser.h"
#include "bluetooth_controller.h"
namespace medex 
{
    namespace hut
    {
        protocol_parser::protocol_parser(bluetooth_controller *bc)
        {
            bc_ = bc;
            df_ = new data_frame();
            connect(bc_, &bluetooth_controller::receive_data_singnal, this, &protocol_parser::receive_data_slot);
        }

        protocol_parser::~protocol_parser()
        {
            if(bc_) 
            {
                delete bc_;
                bc_ = nullptr;
            }
            if(df_)
            {
                delete df_;
                df_ = nullptr;
            }
        }

        void protocol_parser::parse_record_data(uint8_t* data)
        {
            rd_.sys = (uint16_t)(((data[0]) & 0xFF) << 8 | (data[1]) & 0xFF);
            rd_.dia = (uint16_t)(((data[2]) & 0xFF) << 8 | (data[3]) & 0xFF);
            rd_.rate = (uint16_t)(((data[4]) & 0xFF) << 8 | (data[5]) & 0xFF);;
            rd_.year = data[6];
            rd_.mounth = data[7];
            rd_.date = data[8];
            rd_.hour = data[9];
            rd_.min = data[10];
            rd_.ec = data[11];
            rd_.auto_byte = data[12];
            rd_.auto_high = (data[12] & 0xf0) >> 4;
            rd_.auto_low = data[12] & 0x0f;
        }

        void protocol_parser::receive_data_slot(QByteArray value)
        {
            QByteArray hex_array = value.toHex(':');
            const uint8_t* arr = reinterpret_cast<const uint8_t*>(value.data());
            if (df_->parse(arr)) 
            {
                switch (df_->instruction)
                {
                    case 0x44:
                        qDebug() << "instituation is 0x44...";
                        if (df_->data[0] == 0x01)
                        {
                            qDebug() << "set it to be measured every 5 minutes...";
                        }
                        else if (df_->data[0] == 0x00)
                        {
                            qDebug() << "disabled measured every 5 minutes...";
                        }
                    break;
                    case 0x53: // 取得设备记录条数
                        qDebug() << "instituation is 0x53...";
                        record_count_ = (uint16_t)(((df_->data[0]) & 0xFF) << 8 | (df_->data[1]) & 0xFF);
                        qDebug() << "device has "<< record_count_ << " records...";
                    break;
                    case 0x54: // 读取得指定记录数据命令
                        qDebug() << "instituation is 0x54...";
                        parse_record_data(df_->data);
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
