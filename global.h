#ifndef GLOBAL_H
#define GLOBAL_H
#include<QString>
#include<QSerialPort>

struct QSerialPortSettings {
    QString name;
    qint32 baudRate;
    QString stringBaudRate;
    QSerialPort::DataBits dataBits;
    QString stringDataBits;
    QSerialPort::Parity parity;
    QString stringParity;
    QSerialPort::StopBits stopBits;
    QString stringStopBits;
    QSerialPort::FlowControl flowControl;
    QString stringFlowControl;
};
extern QSerialPortSettings g_SerialSetting;
#endif // GLOBAL_H
