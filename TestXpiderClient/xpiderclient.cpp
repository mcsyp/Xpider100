#include "xpiderclient.h"
#include <QHostAddress>
#include <time.h>
#include <xpider_ctl/xpider_protocol.h>
#include <xpider_ctl/xpider_info.h>

#include <stdint.h>

XpiderClient::XpiderClient(QObject *parent) : QObject(parent)
{
  QHostAddress address = QHostAddress::LocalHost;
  host_address_ = address.toString();
  host_port_ = SERVER_PORT;

  connect(&socket_,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
  connect(&socket_,SIGNAL(connected()),this,SLOT(onConnected()));
  connect(&socket_,SIGNAL(disconnected()),this,SLOT(onDisconnected()));

  connect(&timer_retry_,SIGNAL(timeout()),this, SLOT(onRetryTimeout()));
  connect(&timer_hb_,SIGNAL(timeout()),this,SLOT(onHBTimer()));

  connect(&hdlc_encoder_,SIGNAL(hdlcTransmitByte(QByteArray)),this,SLOT(onHdlcTransmitByte(QByteArray)));
  connect(&hdlc_encoder_,SIGNAL(hdlcValidFrameReceived(QByteArray,quint16)),this, SLOT(onHdlcValidFrameReceived(QByteArray,quint16)));
}

void XpiderClient::ConnectToHost(QString address, int port){
  qDebug()<<tr("Connecting to %1:%2").arg(address).arg(port);
  host_address_ = address;
  host_port_ = port;
  timer_retry_.start(CLIENT_RETRY_INTERVAL);
}

void XpiderClient::onDisconnected(){
  qDebug()<<tr("Server fucking disconnected.");
  timer_retry_.start(CLIENT_RETRY_INTERVAL);
  timer_hb_.stop();
}

void XpiderClient::onConnected(){
  qDebug()<<tr("Connected to server.");

  //started test cases
  TestCase1();

  timer_retry_.stop();
  timer_hb_.start(3000);
}

void XpiderClient::onRetryTimeout(){
  qDebug()<<tr("Trying to connect to server:%1,%2").arg(host_address_).arg(host_port_);
  socket_.connectToHost(host_address_,host_port_);
}

void XpiderClient::onHBTimer(){
  qDebug()<<tr("heart beat to xpider");
  socket_.write(QString("fuck it! move!").toUtf8());
}

void XpiderClient::onReadyRead(){
  int len = socket_.bytesAvailable();
  //qDebug()<<tr("[%1,%2] %3 bytes available").arg(__FUNCTION__).arg(__LINE__).arg(len);
  const int rx_size=1024;
  QByteArray rx_payload = socket_.read(rx_size);
  rx_payload.remove(0,2);
  hdlc_encoder_.charReceiver(rx_payload);
}
void XpiderClient::onTimeoutSingle(){

}

void XpiderClient::TestCase0(){

  QString str_message="Fuck you! Loser!";
  str_message.append(QString("I'm %1").arg(socket_.localPort()));
  QByteArray array = str_message.toUtf8();
  hdlc_encoder_.frameDecode(array,array.size());
  qDebug()<<tr("[%1,%2] Sending[%3]: %4")
            .arg(__FUNCTION__).arg(__LINE__)
            .arg(str_message.size())
            .arg(str_message);
}
void XpiderClient::TestCase1(){
  QByteArray tx_pack;
  uint8_t* tx_buffer;
  uint16_t tx_length;
  XpiderInfo info;
  XpiderProtocol  protocol;
  protocol.Initialize(&info);

  //step1.set target angle & transform to tx buffer
  srand(time(NULL));
  info.eye_angle=rand()%60;
  protocol.GetBuffer(protocol.kEye,&tx_buffer,&tx_length);

  //step2. set tx_pack
  tx_pack.append((char*)tx_buffer,tx_length);
  hdlc_encoder_.frameDecode(tx_pack,tx_length);
}

void XpiderClient::onHdlcTransmitByte(QByteArray encoded_data){
  encoded_data.insert(0,QByteArray::fromHex("0155"));
  socket_.write(encoded_data);
}

void XpiderClient::onHdlcValidFrameReceived(QByteArray decoded_data, quint16 frame_size){
  qDebug()<<"rx_message:"<<decoded_data.toHex();
}
