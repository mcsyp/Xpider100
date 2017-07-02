#include "opticlient.h"
#include <QHostAddress>
#include <qdebug.h>
OptiClient::OptiClient(QObject* parent):QTcpSocket(parent)
{
  connect(this,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
  connect(this,SIGNAL(connected()),this, SLOT(onConnected()));
  connect(this,SIGNAL(disconnected()),this,SLOT(onDisconnected()));

  connect(&timer_retry_,SIGNAL(timeout()),this,SLOT(onRetryTimeout()));
  connect(&timer_message_,SIGNAL(timeout()),this,SLOT(onMessageTimeout()));
}

void OptiClient::TryConnectToHost(QString hostname, int port){
  host_name_=hostname;
  host_port_ = port;
  timer_message_.stop();
  timer_retry_.start(TIMEOUT_RETRY);
}

void OptiClient::onConnected(){
  timer_retry_.stop();
  timer_message_.start(TIMEOUT_MESSAGE);
}
void OptiClient::onDisconnected(){
  timer_retry_.start(TIMEOUT_RETRY);
  timer_message_.stop();
}
void OptiClient::onReadyRead(){

}
void OptiClient::onRetryTimeout(){
  this->connectToHost(host_name_,(quint16)host_port_);
  qDebug()<<tr("Trying to connect to %1:%2").arg(host_name_).arg(host_port_);
}
void OptiClient::onMessageTimeout(){
  QByteArray tx_buffer;
  uint8_t head_buffer[64];
  QString tx_message="0.12,0.02,0.03\n1.72,0.06,0.02\n";
  int head_len = protocol_.FillHead(0x9,tx_message.size(),head_buffer,64);

  tx_buffer.append((char*)head_buffer,head_len);
  tx_buffer.append(tx_message);

  this->write(tx_buffer);
}
