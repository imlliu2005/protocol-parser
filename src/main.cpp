#include <QCoreApplication>
#include "bluetooth_controller.h"
#include <iostream>
#include "package_instructions.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

using namespace medex::hut;

void outLogMessageToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString message;
    QString app_run_addr;
    message = qFormatLogMessage(type, context, msg);
    message.append("\r\n");

    //获取程序当前运行目录
    QString current_PathName = QCoreApplication::applicationDirPath();
    if(QFile::exists(current_PathName)==false)
    {
        app_run_addr = "debug.log";
    }else
    {
        app_run_addr = current_PathName + "/"+"debug.log";
    }
    QFile file(app_run_addr);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        file.write(message.toLocal8Bit());
    }
    file.close();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss}] %{file} %{line} %{function} %{message}");
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] %{message}");
    //初始化qdebug的输出重定向到文件
    qInstallMessageHandler(outLogMessageToFile);

    bluetooth_controller * ble_ctrl = new bluetooth_controller();
    // 扫描设备
    ble_ctrl->discover_devices();
    return a.exec();
}
