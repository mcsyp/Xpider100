#include "xpiderclient.h"
#include <QHostAddress>

XpiderClient::XpiderClient(QObject *parent) : QObject(parent)
{
  QHostAddress address = QHostAddress::LocalHost;
  host_address_ = address.toString();
  host_port_ = SERVER_PORT;

  connect(&socket_,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
  connect(&socket_,SIGNAL(connected()),this,SLOT(onConnected()));
  connect(&socket_,SIGNAL(disconnected()),this,SLOT(onDisconnected()));

  connect(&timer_retry_,SIGNAL(timeout()),this, SLOT(onRetryTimeout()));
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
}

void XpiderClient::onConnected(){
  qDebug()<<tr("Connected to server.");

  QString message = "Hello fucking server!";
  SendMessage(SERVER_LOG_REQ,message.toUtf8().data(),message.length()+1);

  TestCase0();
  //socket_.write("hello fucker server?!");
  timer_retry_.stop();
}

int XpiderClient::FillHead(uint16_t cmdid, uint16_t payload_len, uint8_t * buffer, int buffer_size){
  if(buffer==NULL || buffer_size<MESSAGE_HEAD_LEN){
    return 0;
  }
  message_head * temp_ptr = (message_head*)buffer;
  temp_ptr->cmdId = cmdid;
  temp_ptr->len = MESSAGE_HEAD_LEN + payload_len;
  temp_ptr->magic_num1  = MAGIC_NUM1;
  temp_ptr->magic_num2  = MAGIC_NUM2;
  temp_ptr->magic_num3  = MAGIC_NUM3;
  temp_ptr->magic_num4  = MAGIC_NUM4;
  return sizeof(message_head);
}

void XpiderClient::onRetryTimeout(){
  qDebug()<<tr("Trying to connect to server:%1,%2").arg(host_address_).arg(host_port_);
  socket_.connectToHost(host_address_,host_port_);
}


void XpiderClient::onReadyRead(){
  int len = socket_.bytesAvailable();
  qDebug()<<tr("%1 bytes available").arg(len);
  const int max_size=1024;
  char data[max_size];
  socket_.read(data,max_size);
  qDebug()<<tr("message:%1").arg(QString(data));
}

void XpiderClient::SendMessage(int cmdid, char *buffer, int buffer_len){
  char head[MESSAGE_HEAD_LEN];
  //step1. prepare the head
  FillHead(cmdid,buffer_len,(uint8_t*)head,MESSAGE_HEAD_LEN);

  //step2. assemble the pack
  QByteArray array;
  array.append(head,MESSAGE_HEAD_LEN);
  if(buffer && buffer_len>0){
    array.append(buffer,buffer_len);
  }

  //step3. send the pack
  socket_.write(array);
}

void XpiderClient::TestCase0(){
  QString pack_0 = "I will tell you something.";
  QString pack_1 = "Fuck your ass!";
  int test_len = pack_0.size()+pack_1.size();
  qDebug()<<tr("test pack size %1").arg(test_len);

  //step1. prepare the head
  char head[MESSAGE_HEAD_LEN];
  FillHead(SERVER_LOG_REQ,test_len,(uint8_t*)head,MESSAGE_HEAD_LEN);

  //step2. assemble the pack
  QByteArray array;
  array.append(head,MESSAGE_HEAD_LEN);
  array.append(pack_0.toUtf8());

  //step3. send the pack
  socket_.write(array);
  socket_.write(pack_1.toUtf8());
}
