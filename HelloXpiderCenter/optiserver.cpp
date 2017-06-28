#include "optiserver.h"
#include <qdebug.h>
#include <stdio.h>

#define DEBUG 1
ServerOpti::ServerOpti(QObject *parent) :QTcpServer(parent)
{
}
ServerOpti::~ServerOpti(){
  ResetServer();
}
void ServerOpti::ResetServer(){
  if(this->isListening()){
    //stop server and reset
    this->close();
    printf("[Xpider] server stoped.\n");
  }
  threadpool_.clear();
  clientlist_.clear();
}

int ServerOpti::StartServer(){
  //step1. reset server
  ResetServer();

  //step2. start listening
  this->listen(QHostAddress::Any,SERVER_PORT);
  threadpool_.setMaxThreadCount(SERVER_MAX_THREADPOOL);

  printf("[Xpider] server started on %d, threadpool size %d\n",SERVER_PORT, threadpool_.maxThreadCount());
  return this->serverPort();
}

void ServerOpti::incomingConnection(qintptr socket){
  printf("[Xpider] new client socket received %d\n",socket);

  //step1. call the base incomming socket
  //QTcpServer::incomingConnection(socket);

  //step2. init the socket
  ClientOpti * xpider = new ClientOpti;
  xpider->socketDescriptor_ = socket;
  xpider->server_ = this;
  clientlist_.push_back(xpider);//save the xpider to client list

  //step3. start the runnable
  threadpool_.start(xpider);
}

int ServerOpti::FindMessageHead(uint8_t *rx_buffer, int rx_len, int out_list[],int out_size){
  int out_counter=0;
  if(out_list==NULL || out_size<=0){
    return 0;
  }

  for(int i=0;i<rx_len-MESSAGE_HEAD_LEN;++i){
    if(CheckMessageHead(rx_buffer+i,MAGIC_NUM_LEN)){
      if(out_counter<out_size){//save the output list
        out_list[out_counter] = i;
        ++out_counter;
      }
    }
  }
  return out_counter;
}
bool ServerOpti::CheckMessageHead(uint8_t * rx_buffer, int rx_len){
  if(rx_buffer==NULL || rx_len<MAGIC_NUM_LEN){
    return false;
  }
  return (rx_buffer[0]==MAGIC_NUM1 &&
             rx_buffer[1]==MAGIC_NUM2 &&
             rx_buffer[2]==MAGIC_NUM3 &&
             rx_buffer[3]==MAGIC_NUM4);
}

int ServerOpti::FillHead(uint16_t cmdid, uint16_t payload_len, uint8_t * buffer, int buffer_size){
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

bool ServerOpti::RemoveClient(ClientOpti *client){
  for(auto iter=clientlist_.begin();iter!=clientlist_.end();++iter){
    ClientOpti * item = *iter;
    if(item==client){
      clientlist_.erase(iter);
      return true;
    }
  }
  return false;
}

void ClientOpti::run(){
  QTcpSocket socket;
  char rx_buffer[RX_MAX_SIZE];

  //step1. setup socket reset worklist
  socket.setSocketDescriptor(socketDescriptor_);

  //step2. register to server
  Reset();

  //setp3. loop for messages
  while(socket.state()!=QTcpSocket::UnconnectedState){

    //step1. process rx data
    if(socket.waitForReadyRead(RX_TIMEOUT)){
      //if message arrives
      int rx_len = socket.read(rx_buffer,RX_MAX_SIZE);
      //printf("[%s,%d] rx message is :%s\n",__FUNCTION__,__LINE__,rx_buffer);
      //process the rx raw data
      RxProcess((uint8_t*)rx_buffer,rx_len);
    }

    //step2. process tx tasks
    if(!tx_queue_.isEmpty()){
      //if tx queue is not empty, let socket output it.
      for(int i=0;i<tx_queue_.size();++i){
        const QByteArray array  =  tx_queue_.at(i);
        socket.write(array);
      }
      tx_queue_.clear();//clear the whole area
    }

    //printf("[%s,%d] client %d alive.\n",__FILE__,__LINE__,socketDescriptor_);
  }

  //step4. un-register from server
  if(server_ && server_->RemoveClient(this)){
    printf("[%s,%d] client %d removed.\n",__FILE__,__LINE__,socketDescriptor_);
  }
  printf("[Xpider] client socket %d disconneceted\n",socketDescriptor_);
}

void ClientOpti::Reset(){
  tx_queue_.clear();
  rx_payload_.clear();

  rx_state_ = RxStateIdle;
  rx_payload_len_=0;
  rx_payload_size_=0;
}

void ClientOpti::RxProcess(uint8_t *data, int len){
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
        if(i<=len-ServerOpti::MESSAGE_HEAD_LEN &&
           ServerOpti::CheckMessageHead(current,ServerOpti::MAGIC_NUM_LEN)){

          //step1.fill the head
           memcpy((uint8_t*)&rx_message_head_,current,sizeof(rx_message_head_));
           rx_payload_size_ = rx_message_head_.len-ServerOpti::MESSAGE_HEAD_LEN;
           rx_payload_.clear();//clear the rx payload

           //step2.update  the state
           rx_state_ = RxStateProcessing;

           //step3. i jump to payload
           delta = ServerOpti::MESSAGE_HEAD_LEN;
        }
        break;
    }
    //printf("[%s,%d] rx_state=%d,delta=%d, payload_size\n",__FUNCTION__,__LINE__,rx_state_,delta,rx_payload_size_);
    if(rx_payload_.size()>=rx_payload_size_){
      //check if a package is full
      RxProcessPayload(&rx_message_head_,rx_payload_);
      rx_state_ = RxStateIdle;
    }

    //increase
    i=i+delta;
  }
}
void ClientOpti::RxProcessPayload(ServerOpti::message_head *head, QByteArray &payload){
   printf("[%s,%d]cmdid=%d, len=%d, payload_size=%d, payload=%s\n",
          __FUNCTION__,__LINE__,
          head->cmdId,
          head->len,
          payload.size(),
          payload.data());
}
