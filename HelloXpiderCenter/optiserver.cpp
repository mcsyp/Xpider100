#include "optiserver.h"
#include <qdebug.h>
#include <stdio.h>

OptiServer::OptiServer(QObject *parent) :QTcpServer(parent)
{
  client_=NULL;
  connect(this,SIGNAL(newConnection()),this,SLOT(onNewConnection()));
}
OptiServer::~OptiServer(){
  ResetServer();
}
void OptiServer::ResetServer(){
  //reset client
  if(client_)client_->disconnectFromHost();
  client_=NULL;

  //reset server
  if(this->isListening()){
    //stop server and reset
    this->close();
    printf("[%s,%d] Opti server stoped.\n",__FUNCTION__,__LINE__);
  }

  //reset rx message state
  rx_payload_.clear();
  rx_state_ = RxStateIdle;
  rx_payload_size_=0;
}

int OptiServer::StartServer(){
  //step1. reset server
  ResetServer();

  //step2. start listening
  this->listen(QHostAddress::Any,SERVER_PORT);

  printf("[%s,%d] Opti server started on %d\n",__FUNCTION__,__LINE__,SERVER_PORT);
  return this->serverPort();
}

void OptiServer::onNewConnection(){
  QTcpSocket *ptr = this->nextPendingConnection();
  if(client_!=NULL){
    client_->disconnectFromHost();
  }
  client_ = ptr;
  connect(client_,SIGNAL(readyRead()),this, SLOT(onClientReadyRead()));
  connect(client_,SIGNAL(disconnected()),this,SLOT(onClientDisconnected()));
  client_->write("Hey bro, opti server connected.\n");
}

void OptiServer::onClientDisconnected(){
  if(client_)client_=NULL;
}

void OptiServer::onClientReadyRead(){
  QByteArray rx_raw = client_->read(RX_MAX_SIZE);
  RxProcess((uint8_t*)rx_raw.data(),rx_raw.size());
  printf("[%s,%d] rx_message length:%d\n",__FILE__,__LINE__,rx_raw.size());
}

void OptiServer::RxProcess(uint8_t *data, int len){
  int i=0;
  while(i<len){
    uint8_t * current = data+i;
    int delta=1;
    switch(rx_state_){
    case RxStateProcessing:
        //if there is a previous head
        if(rx_payload_.size()<rx_payload_size_){
          rx_payload_.push_back((char)data[i]);
        }
        break;
    case RxStateIdle:
        //step1. check if there is are head since this byte
        if(i<=len-OptiServer::MESSAGE_HEAD_LEN &&
           CheckMessageHead(current,OptiServer::MAGIC_NUM_LEN)){

          //step1.fill the head
           memcpy((uint8_t*)&rx_message_head_,current,sizeof(rx_message_head_));
           rx_payload_size_ = rx_message_head_.len-OptiServer::MESSAGE_HEAD_LEN;
           rx_payload_.clear();//clear the rx payload

           //step2.update  the state
           rx_state_ = RxStateProcessing;

           //step3. i jump to payload
           delta = MESSAGE_HEAD_LEN;
        }
        break;
    }

    if(rx_payload_.size()>=rx_payload_size_){
      //check if a package is full
      RxProcessPayload(&rx_message_head_,rx_payload_);
      rx_state_ = RxStateIdle;
    }

    //increase
    i=i+delta;
  }
}

void OptiServer::RxProcessPayload(OptiServer::message_head *head, QByteArray &payload){
  printf("[%s,%d]rx payalod:%d\n",__FILE__,__LINE__,head->cmdId);
  printf("%s",payload.data());
}

bool OptiServer::CheckMessageHead(uint8_t * rx_buffer, int rx_len){
  if(rx_buffer==NULL || rx_len<MAGIC_NUM_LEN){
    return false;
  }
  return (rx_buffer[0]==MAGIC_NUM1 &&
          rx_buffer[1]==MAGIC_NUM2 &&
          rx_buffer[2]==MAGIC_NUM3 &&
          rx_buffer[3]==MAGIC_NUM4);
}

int OptiServer::FillHead(uint16_t cmdid, uint16_t payload_len, uint8_t * buffer, int buffer_size){
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

