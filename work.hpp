#ifndef WORK_H
#define WORK_H
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QSerialPort>
#include "gui/settingsdialog.h"
#include "global.h"

class SerialPortWorker : public QObject
{
    Q_OBJECT
public slots:
    void doWrite(const QByteArray &writeData) {

        if(!serial.isOpen()){
            QObject::connect(&serial, &QSerialPort::readyRead, this, &SerialPortWorker::readyReadSlot); //多线程写
        }else{
            qint64 size =serial.write(writeData.data(),writeData.length());
            Q_UNUSED(size);
            if(size==writeData.length()){
                qDebug()<<"write...ok";
                //                emit resultReady(result);
            }
        }
    }
    void readyReadSlot()
    {
        //        qDebug()<<__FUNCTION__<<QThread::currentThreadId();
        QByteArray readData;
        while (!serial.atEnd()) {
            readData += serial.read(1024);
        }
        //        if(serial.waitForReadyRead(-1)){
        //            readData += serial.readAll();
        //        }
        if(readData.length()>=0){
            //            qDebug()<<"read...:"<<readData.toHex().data();
            //            qDebug()<<"read...:"<<readData.length();
            qint32 index =-1;
            QByteArrayList list= readData.split('\n');
            //            开始解析 $******\r\n类型的数据
            //分割出一条完整报文，并转发出去，使用引用模式传参未发生数据拷贝操作
            for(int i=0;i<list.size();i++){
                if(list.at(i).length() && (list.at(i)[0] =='$') )
                {
                    index =list.at(i).indexOf("*");
                    if(index){
                        list[i].remove(index,3);
                    }
                    emit doRead(list.at(i)); //list.at(i) const类型
                }
            }
        }
    }
    void openSerialPort()
    {
        p= g_SerialSetting;
        serial.setPortName(p.name);
        serial.setBaudRate(p.baudRate);
        serial.setDataBits(p.dataBits);
        serial.setParity(p.parity);
        serial.setStopBits(p.stopBits);
        serial.setFlowControl(p.flowControl);
        if (serial.open(QIODevice::ReadWrite)) {
            connect(&serial,&QSerialPort::readyRead,this,&SerialPortWorker::readyReadSlot);
            //            qDebug()<<"serial->open success";
        } else {
            //            qDebug()<<"serial->open failed";
        }
    }
    void closeSerialPort()
    {
        if (serial.isOpen())
            serial.close();
    }

signals:
    void doRead(const QByteArray &result);      //读取的数据,若解析时需要修改数据，先进行数据拷贝
    void resultReady(const QByteArray &result); //返回需要执行的结果，或者其他错误信息
private:
    QSerialPort serial;
    QSerialPortSettings p;
};

class Controller : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller() {
        SerialPortWorker *worker = new SerialPortWorker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

        connect(this, &Controller::openSerialPort, worker, &SerialPortWorker::openSerialPort); //open
        connect(this, &Controller::closeSerialPort, worker, &SerialPortWorker::closeSerialPort); //close
        connect(this, &Controller::doWrite, worker, &SerialPortWorker::doWrite); //多线程写
        connect(worker, &SerialPortWorker::doRead, this, &Controller::doRead);//多线程返回

        connect(worker, &SerialPortWorker::resultReady, this, &Controller::handleResults);//多线程返回
        workerThread.start();
    }
    ~Controller() {
        workerThread.quit();
        workerThread.wait();
    }
public slots:

signals:
    void doWrite(const QByteArray &);
    void doRead(const QByteArray &);
    void handleResults(const QByteArray &);
    void openSerialPort();
    void closeSerialPort();
};

#endif // WORK_H
