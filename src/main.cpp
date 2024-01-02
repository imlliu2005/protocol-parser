#include <QCoreApplication>
#include "bluetooth_controller.h"
using namespace medex::hut;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    bluetooth_controller * ble_ctrl = new bluetooth_controller();
    // 扫描设备
    ble_ctrl->discover_devices();
    // 连接设备
    QTimer::singleShot(2000, [=](){
        ble_ctrl->create_ble_controller();
        ble_ctrl->connect_device();
    });

    return a.exec();
}
