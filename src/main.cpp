#include <QCoreApplication>
#include<QTimer>
#include <QDebug>
#include "bluetooth_controller.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    bluetooth_controller * ble_ctrl = new bluetooth_controller();
    // 扫描设备
    ble_ctrl->discover_devices();
    // 连接设备
    QTimer::singleShot(3000, [=](){
        ble_ctrl->create_ble_controller();
        ble_ctrl->connect_device();
    });

    return a.exec();
}
