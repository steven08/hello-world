#include "mytcpserver.h"
#include "ui_mytcpserver.h"
#include "modbus_rtu.h"

MyTcpServer::MyTcpServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyTcpServer)
{
    ui->setupUi(this);

    tcpServer = new QTcpServer(this);
    ui->edtIP->setText(QNetworkInterface().allAddresses().at(1).toString());   //获取本地IP
    ui->btnConnect->setEnabled(true);
    //ui->btnSend->setEnabled(false);
    ui->btnSend->setEnabled(true);

    //debug
    ui->lineEdit_client->setText("01");
    ui->lineEdit_func->setText("03");
    ui->lineEdit_address->setText("1000");
    ui->lineEdit_count->setText("5");

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(NewConnectionSlot()));
}

MyTcpServer::~MyTcpServer()
{
    delete ui;
}
// newConnection -> newConnectionSlot 新连接建立的槽函数
void MyTcpServer::NewConnectionSlot()
{
    currentClient = tcpServer->nextPendingConnection();
    tcpClient.append(currentClient);
    ui->cbxConnection->addItem(tr("%1:%2").arg(currentClient->peerAddress().toString().split("::ffff:")[1])\
                                          .arg(currentClient->peerPort()));
    connect(currentClient, SIGNAL(readyRead()), this, SLOT(ReadData()));
    connect(currentClient, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
}

// 客户端数据可读信号，对应的读数据槽函数
void MyTcpServer::ReadData()
{
    // 由于readyRead信号并未提供SocketDecriptor，所以需要遍历所有客户端
    for(int i=0; i<tcpClient.length(); i++)
    {
        QByteArray buffer = tcpClient[i]->readAll();
        if(buffer.isEmpty())    continue;

        static QString IP_Port, IP_Port_Pre;
        IP_Port = tr("[%1:%2]:").arg(tcpClient[i]->peerAddress().toString().split("::ffff:")[1])\
                                     .arg(tcpClient[i]->peerPort());

        // 若此次消息的地址与上次不同，则需显示此次消息的客户端地址
        if(IP_Port != IP_Port_Pre)
            ui->edtRecv->append(IP_Port);

        ui->edtRecv->append(buffer);
#if 1
        //将接收到的数据解析，并且在显示窗口显示出来
        modbus_rtu mr;
        modbus_read_reg_info_st reg_info;
        reg_info.func_code = E_FUNC_CODE_READ_HOLDING_REGISTERS;
        reg_info.slave_addr = 1;
        reg_info.register_cnt = 5;//16 bit storage 8 bit, so half of the raw.
        reg_info.register_addr = 1000;

        modbus_rtu_reg_data_val_st data_val;
        int32_t count = buffer.size();
        const int max_count = 1024;
        uint8_t hex[max_count];
        memset(hex, 0, max_count);
        memcpy(hex,buffer.constData(),static_cast<size_t>(count));
        mr.modbus_rtu_unpack_read_register(&reg_info, hex, static_cast<uint32_t>(count), &data_val);

        QString showTxt;
        showTxt.append("there are ");
        int num = data_val.read_register_cnt;// 1000 = 0x03E8;
        QString  str = QString("%1").arg(num,4,16,static_cast<QChar>('0')).toUpper();
        showTxt.append(str);
        showTxt.append(": \n");
        for (int i = 0; i < data_val.read_register_cnt; i++) {
            int reg_value = data_val.reg_data.storage_reg_value[i];
            QString  str = QString("%1").arg(reg_value,4,16,static_cast<QChar>('0')).toUpper();
            showTxt.append(str);
            showTxt.append("\n");
        }
        showTxt.append("end\n");
        ui->edtRecv->append(showTxt);
#endif
        //更新ip_port
        IP_Port_Pre = IP_Port;
    }
}
// disconnected -> disconnectedSlot 客户端断开连接的槽函数
void MyTcpServer::disconnectedSlot()
{
    //由于disconnected信号并未提供SocketDescriptor，所以需要遍历寻找
    for(int i=0; i<tcpClient.length(); i++)
    {
        if(tcpClient[i]->state() == QAbstractSocket::UnconnectedState)
        {
            // 删除存储在combox中的客户端信息
            ui->cbxConnection->removeItem(ui->cbxConnection->findText(tr("%1:%2")\
                                  .arg(tcpClient[i]->peerAddress().toString().split("::ffff:")[1])\
                                  .arg(tcpClient[i]->peerPort())));
            // 删除存储在tcpClient列表中的客户端信息
             tcpClient[i]->destroyed();
             tcpClient.removeAt(i);
        }
    }
}
// 监听--断开
void MyTcpServer::on_btnConnect_clicked()
{
    if(ui->btnConnect->text()=="监听")
    {
        bool ok = tcpServer->listen(QHostAddress::Any, ui->edtPort->text().toInt());
        if(ok)
        {
            ui->btnConnect->setText("断开");
            ui->btnSend->setEnabled(true);
        }
    }
    else
    {
        for(int i=0; i<tcpClient.length(); i++)//断开所有连接
        {
            tcpClient[i]->disconnectFromHost();
            bool ok = tcpClient[i]->waitForDisconnected(1000);
            if(!ok)
            {
                // 处理异常
            }
            tcpClient.removeAt(i);  //从保存的客户端列表中取去除
        }
        tcpServer->close();     //不再监听端口
        ui->btnConnect->setText("监听");
        ui->btnSend->setEnabled(false);
    }
}

bool MyTcpServer::checkModbusGroup()
{
    QString func_data = ui->lineEdit_func->text();
    QString client_data = ui->lineEdit_count->text();
    QString count_data = ui->lineEdit_count->text();
    QString address_data = ui->lineEdit_address->text();

    if (func_data.isNull() || client_data.isNull()
                    || count_data.isNull()
                    || address_data.isNull()){
        return false;
    } else {
        return true;
    }
}

//read data from the modbus group, and transfer with modbus
//protocl
QByteArray MyTcpServer::getModbusGroupData()
{
    QString func_data = ui->lineEdit_func->text();
    QString client_data = ui->lineEdit_client->text();
    QString count_data = ui->lineEdit_count->text();
    QString address_data = ui->lineEdit_address->text();

    modbus_read_reg_info_st modbus_read_reg_info;
    //slave address
    QByteArray slave_addr = client_data.toLatin1();
    modbus_read_reg_info.slave_addr= slave_addr.toShort();
    //func_code
//    QByteArray func_code = func_data.toLatin1();
    modbus_read_reg_info.func_code = E_FUNC_CODE_READ_HOLDING_REGISTERS;
    //address
    QByteArray address = address_data.toLatin1();
    modbus_read_reg_info.register_addr= address_data.toShort();
    //count.
    QByteArray count = count_data.toLatin1();
    modbus_read_reg_info.register_cnt= count.toShort();

    //using modbus_rtu to transfroming to stream.
    modbus_rtu mr;
    uint8_t buf[1024];
    memset(buf, 0, 1024);
    uint32_t data_len = 0;
    //mr.modbus_rtu_pack_read_reg(&modbus_read_reg_info, (uint8_t*)buf, &data_len);
    mr.modbus_rtu_pack_read_reg(&modbus_read_reg_info, (uint8_t*)buf, &data_len);
    QByteArray byteArray;
    for(int i = 0; i < data_len; i++) {
        byteArray.append(buf[i]);
    }

    return byteArray;
}

// 发送数据
void MyTcpServer::on_btnSend_clicked()
{
    QString data = ui->edtSend->toPlainText();
    bool bReady = checkModbusGroup();
    ui->modbus_show->setText(tr("你的数据没有设置好"));
    if(data == "" && !bReady) {
        ui->modbus_show->setText(tr("你的数据没有设置好"));
        return;    // 文本输入框为空时
    }

    //对Modbus组件的数据提取
    QByteArray sendData = getModbusGroupData();

    //全部连接
//    QByteArray sendData = data.toLatin1();
    if(ui->cbxConnection->currentIndex() == 0)
    {
        for(int i=0; i<tcpClient.length(); i++)
            tcpClient[i]->write(sendData); //qt5除去了.toAscii()
    }
    //指定连接
    else
    {
        QString clientIP = ui->cbxConnection->currentText().split(":")[0];
        int clientPort = ui->cbxConnection->currentText().split(":")[1].toInt();
        //        qDebug() << clientIP;
        //        qDebug() << clientPort;
        for(int i=0; i<tcpClient.length(); i++)
        {
            if(tcpClient[i]->peerAddress().toString().split("::ffff:")[1]==clientIP\
                            && tcpClient[i]->peerPort()==clientPort)
            {
                tcpClient[i]->write(sendData);
                return; //ip:port唯一，无需继续检索
            }
        }
    }
}

void MyTcpServer::requestPowerParameter()
{
    QByteArray cmd_head("01");
    QByteArray cmd_("01");
    QByteArrayData requestCommand;
}

void MyTcpServer::on_btnClear_clicked()
{
    ui->edtRecv->clear();
}

