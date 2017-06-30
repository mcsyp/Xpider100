#include "xpiderclient.h"

#include <QTcpSocket>
#include <QTime>
#include <QThread>
#include <xpider_ctl/xpider_info.h>
#include <xpider_ctl/xpider_linked_list.h>
#include <xpider_ctl/xpider_protocol.h>

#define MESSAGE_HEAD "0155"
const QByteArray XpiderClient::XPIDER_MESSAGE_HEAD = QByteArray::fromHex(MESSAGE_HEAD);
XpiderClient::XpiderList XpiderClient::g_xpider_list_ = XpiderClient::XpiderList();
XpiderClient::XpiderClient(){
  ptr_socket_=NULL;
  ptr_time_ = NULL;
  ptr_hdlc_ = NULL;
}
XpiderClient* XpiderClient::Create(QString host_name, int port){
  if(host_name.size()==0 || port<=0)return NULL;

  XpiderClient * client = new XpiderClient;
  client->host_name_ = host_name;
  client->host_port_ = port;

  g_xpider_list_.push_back(client);

  return client;
}

void XpiderClient::RemoveInstance(XpiderClient *instance){
  for(auto iter=g_xpider_list_.begin();iter!=g_xpider_list_.end();++iter){
    XpiderClient * xpider = *iter;
    if(xpider==instance){
      g_xpider_list_.erase(iter);
      break;
    }
  }
}

void XpiderClient::DisposeAllClients()
{
  for(auto iter=g_xpider_list_.begin();iter!=g_xpider_list_.end();++iter){
    XpiderClient * xpider = *iter;
    if(xpider){
      xpider->is_running_=false;
      QThread::sleep(200);//wait for stop
    }
  }
  g_xpider_list_.clear();
}

void XpiderClient::run(){
  QTime time;
  QTcpSocket socket;
  XpiderHdlcEncoder hdlc_encoder(this);
  QByteArray rx_message;

  //step2.reset and conennect to server
  Reset();
  is_running_ = true;
  is_alive_ = true;
  ptr_socket_ = &socket;
  ptr_hdlc_ = &hdlc_encoder;
  ptr_time_ = &time;
  time.start();

  printf("[%s,%d] %s:%d xpider_thread startd\n",
         __FUNCTION__,__LINE__,
         host_name_.toLatin1().data(),
         host_port_);

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
        ConnectionHeartBeat(time.elapsed(),socket);

        break;
    }

    QThread::yieldCurrentThread();
  }

  //if thread stoped
  if(socket.state()!=QTcpSocket::UnconnectedState){
    socket.disconnectFromHost();
    socket.waitForDisconnected(10000);
  }
  RemoveInstance(this);

  printf("Xpider[%s] connection thread exit.\n",host_name_.toLatin1().data());
}

void XpiderClient::Reset(){
  tx_queue_.clear();
  is_running_ = false;
  is_alive_ =false;
}

void XpiderClient::onDecodedMessage(QByteArray &dec_message, quint16 dec_length){
#if 1
  if(dec_length) qDebug()<<"rx_message:"<<dec_message.toHex();
#else
  static int last_msec=0;
  static const int HB_TIMEOUT=10000;//10sec

  if(ptr_time_==NULL)return;

  int current_time = ptr_time_->elapsed();
  bool state = ((current_time-last_msec)<HB_TIMEOUT);
  last_msec = current_time;
  if(state!=is_alive_){
    printf("[%s,%d]xpider[%s] is %d\n",
           __FUNCTION__,__LINE__,
           host_name_.toLatin1().data(),
           state);
  }
  is_alive_ = state;
#endif
}
void XpiderClient::onEncodedMessage(QByteArray &enc_message){
  QByteArray tx_payload;
  tx_payload.append(XPIDER_MESSAGE_HEAD);
  tx_payload.append(enc_message);
  if(ptr_socket_){
    ptr_socket_->write(tx_payload);
  }
}

void XpiderClient::XpiderConnectedAction()
{
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
  if(ptr_hdlc_)ptr_hdlc_->hdlc_.frameDecode(tx_pack,tx_length);
}

void XpiderClient::ConnectionRetry(int current_msec, QTcpSocket &socket){
  static int last_msec=0;
  if(current_msec-last_msec<RETRY_INTERVAL){ return;}

  if(socket.state()!=QTcpSocket::ConnectingState){
    socket.connectToHost(host_name_,host_port_);
    //printf("[%s,%d]Trying to connect to %s:%d\n",__FILE__,__LINE__,host_name_.toLatin1().data(),host_port_);
  }
  if(socket.waitForConnected(3000)){
    XpiderConnectedAction();
    printf("Xpider[%s] connected\n",host_name_.toLatin1().data());
  }

  //last step;
  last_msec = current_msec;
}

void XpiderClient::ConnectionHeartBeat(int current_msec, QTcpSocket &socket){
  static int last_msec=0;
  if(current_msec-last_msec<HB_INTERVAL){ return;}

  QByteArray payload;

  //printf("[%s,%d] tx hb\n",__FUNCTION__,__LINE__);
  payload=QString("    ").toUtf8();
  payload.insert(0,XPIDER_MESSAGE_HEAD);
  socket.write(payload);

  //last step;
  last_msec = current_msec;
}


