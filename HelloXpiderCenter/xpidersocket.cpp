#include "xpidersocket.h"
#include "time.h"

#include <QTcpSocket>
#include <QTime>
#include <QThread>
#include <stdio.h>
#include "xpider_ctl/xpider_info.h"
#include "xpider_ctl/linked_list.h"
#include "xpider_ctl/xpider_protocol.h"

#define MESSAGE_HEAD "0155"
const QByteArray XpiderSocket::XPIDER_MESSAGE_HEAD = QByteArray::fromHex(MESSAGE_HEAD);
XpiderSocket::XpiderMap XpiderSocket::g_xpider_map_ = XpiderSocket::XpiderMap();

XpiderSocket::XpiderSocket(){
  ptr_socket_=NULL;
  ptr_time_ = NULL;
  ptr_hdlc_ = NULL;
  xpider_id_= 0;
  last_alive_tiggered_ = 0;
}

XpiderSocket* XpiderSocket::Create(QString host_name, int port,XpiderInstance* instance){
  if(host_name.size()==0 || port<=0)return NULL;

  XpiderSocket * client = new XpiderSocket;
  client->host_name_ = host_name;
  client->host_port_ = port;
  client->xpider_id_ = g_xpider_map_.size();
  client->xpider_event_ = instance;

  g_xpider_map_[client->xpider_id_] = client;

  return client;
}

void XpiderSocket::Dispose(XpiderSocket *instance){
  for(auto iter = g_xpider_map_.begin();iter!=g_xpider_map_.end();++iter){
    XpiderSocket * xpider = iter->second;
    if(xpider==instance){
      g_xpider_map_.erase(iter);
      xpider->is_running_=false;
      break;
    }
  }
}

void XpiderSocket::DisposeAll(){
  for(auto iter=g_xpider_map_.begin();iter!=g_xpider_map_.end();++iter){
    XpiderSocket * xpider = iter->second;
    if(xpider){
      xpider->is_running_=false;
    }
  }
  g_xpider_map_.clear();
}
XpiderSocket* XpiderSocket::Search(uint32_t id){
  if(g_xpider_map_.count(id)){
    return g_xpider_map_[id];
  }else{
    return NULL;
  }
}

XpiderSocket::~XpiderSocket(){
  Dispose(this);
}

void XpiderSocket::run(){
  QTime time;
  QTcpSocket socket;
  XpiderHdlcEncoder hdlc_encoder(this);
  QByteArray rx_message;

  //step2.reset and conennect to server
  Reset();
  is_running_ = true;
  ptr_socket_ = &socket;
  ptr_hdlc_ = &hdlc_encoder;
  ptr_time_ = &time;

  is_alive_=false;
  last_alive_tiggered_ = 0 ;

  time.start();

  printf("[%s,%d] %s:%d xpider_thread startd. Xpider ID:0x%x\n",
         __FUNCTION__,__LINE__,
         host_name_.toLatin1().data(),
         host_port_,
         xpider_id_);

  //step3. start loop event
  while(is_running_){
    switch(socket.state()){
    case QTcpSocket::UnconnectedState:
        //if not connected, try to connect
        ConnectionRetry(time.elapsed(),socket);
        break;
    default:
        //step1.if conneted, process rx message and tx queue
        if(socket.waitForReadyRead(RX_TIMEOUT)){
          //if message arrives
          rx_message = socket.read(RX_MAX_SIZE);
          rx_message.remove(0,XPIDER_MESSAGE_LEN);
          //printf("[%s,%d] rx_message length:%d\n",__FUNCTION__,__LINE__,rx_message.size());
          //process the rx raw data
          hdlc_encoder.hdlc_.charReceiver(rx_message);
        }

        //step2. process tx tasks
        if(!tx_queue_.isEmpty()){
          //if tx queue is not empty, let socket output it.
          for(int i=0;i<tx_queue_.size();++i){
            const QByteArray array  =  tx_queue_.at(i);
            hdlc_encoder.hdlc_.frameDecode(array,array.size());//encoded
          }
          tx_queue_.clear();//clear the whole area
        }

        //step3. task area
        //send heart beat to xpider.
        ConnectionTxHeartBeat(time.elapsed(),socket);

        break;
    }
    if(this->xpider_event_)this->xpider_event_->SocketStateUpdate(socket.state());

    QThread::yieldCurrentThread();
  }

  //if thread stoped
  if(socket.state()!=QTcpSocket::UnconnectedState){
    socket.disconnectFromHost();
    socket.waitForDisconnected(10000);
  }
  Dispose(this);

  printf("Xpider[%s] connection thread exit.\n",host_name_.toLatin1().data());
}

void XpiderSocket::Reset(){
  tx_queue_.clear();
  is_running_ = false;
}

void XpiderSocket::onDecodedMessage(QByteArray &dec_message, quint16 dec_length){
  //if(dec_length) qDebug()<<"rx_message:"<<dec_message.toHex();
  static const int HB_TIMEOUT=5000;//10sec
  if(ptr_time_==NULL)return;
  int current_time = ptr_time_->elapsed();
  bool state = ((current_time-last_alive_tiggered_)<HB_TIMEOUT);
  last_alive_tiggered_ = current_time;
  is_alive_ = state;
}
void XpiderSocket::onEncodedMessage(QByteArray &enc_message){
  QByteArray tx_payload;
  tx_payload.append(XPIDER_MESSAGE_HEAD);
  tx_payload.append(enc_message);
  if(ptr_socket_){
    ptr_socket_->write(tx_payload);
  }
}

void XpiderSocket::TestingAction()
{
  QByteArray tx_pack;
  uint8_t* tx_buffer;
  uint16_t tx_length;
  XpiderInfo info;
  XpiderProtocol  protocol;
  protocol.Initialize(&info);

  //step1.set target angle & transform to tx buffer
  // srand(time(NULL));
  // info.eye_angle=rand()%60;
  // protocol.GetBuffer(protocol.kEye,&tx_buffer,&tx_length);
  info.rotate_speed = 70;
  info.rotate_rad = -2.53;
  info.walk_speed = 70;
  info.walk_step = 3;
  protocol.GetBuffer(protocol.kAutoMove, &tx_buffer, &tx_length);

  //step2. set tx_pack
  tx_pack.append((char*)tx_buffer,tx_length);
  //if(ptr_hdlc_)ptr_hdlc_->hdlc_.frameDecode(tx_pack,tx_length);
  AppendTxMessage(tx_pack);
}

void XpiderSocket::AppendTxMessage(QByteArray &tx_message){
  tx_queue_.push_back(tx_message);
}

void XpiderSocket::ConnectionRetry(int current_msec, QTcpSocket &socket){
  static int retry_last_msec=0;
  if(current_msec-retry_last_msec<RETRY_INTERVAL){ return;}

  if(socket.state()!=QTcpSocket::ConnectingState){
    socket.connectToHost(host_name_,host_port_);
    //printf("[%s,%d]Trying to connect to %s:%d\n",__FILE__,__LINE__,host_name_.toLatin1().data(),host_port_);
  }
  if(socket.waitForConnected(3000)){
    //TestingAction();
    printf("Xpider[%s] connected\n",host_name_.toLatin1().data());
  }

  //last step;
  retry_last_msec = current_msec;
}

void XpiderSocket::ConnectionTxHeartBeat(int current_msec, QTcpSocket &socket){
  static int hb_last_msec=0;
  if(current_msec-hb_last_msec<HB_INTERVAL){ return;}

  QByteArray payload;

  //printf("[%s,%d] tx hb\n",__FUNCTION__,__LINE__);
  payload=QString("    ").toUtf8();
  payload.insert(0,XPIDER_MESSAGE_HEAD);
  socket.write(payload);

  //last step;
  hb_last_msec = current_msec;
}


