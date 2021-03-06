#include "xpidersocketthread.h"
#include "xpider_ctl/xpider_info.h"
#include "xpider_ctl/linked_list.h"
#include "xpider_ctl/xpider_protocol.h"

#include <time.h>

#define MESSAGE_HEAD "0155"
const QByteArray XpiderSocketThread::XPIDER_MESSAGE_HEAD = QByteArray::fromHex(MESSAGE_HEAD);
XpiderSocketThread::XpiderMap XpiderSocketThread::g_xpider_map_ = XpiderSocketThread::XpiderMap();
void XpiderSocketThread::Remove(uint32_t id){
  for(auto iter = g_xpider_map_.begin();iter!=g_xpider_map_.end();++iter){
    if(iter->first==id){
      g_xpider_map_.erase(iter++);
      break;
    }
  }
}
XpiderSocketThread* XpiderSocketThread::Socket(uint32_t id){
  if(g_xpider_map_.count(id)){
    return g_xpider_map_[id];
  }else{
    return NULL;
  }
}

XpiderSocketThread::XpiderSocketThread(QObject* parent):QTcpSocket(parent){
  connect(&hdlc_,SIGNAL(hdlcTransmitByte(QByteArray)),this, SLOT(onHdlcEncodedByte(QByteArray)));
  connect(&hdlc_,SIGNAL(hdlcValidFrameReceived(QByteArray,quint16)),this,SLOT(onHdlcDecodedByte(QByteArray,quint16)));

  //connect(&timer_retry_,SIGNAL(timeout()),this,SLOT(onTimeoutRetry()));

  connect(this,SIGNAL(connected()),this,SLOT(onConnected()));
  connect(this,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
  connect(this,SIGNAL(readyRead()),this,SLOT(onReadyRead()));

  //init id
  my_id_ = g_xpider_map_.size();
  g_xpider_map_[my_id_] = this;

  //init event thread
  moveToThread(&event_thread_);
}
XpiderSocketThread::~XpiderSocketThread(){
  Remove(my_id_);
  disconnectFromHost();

  event_thread_.exit(0);
  event_thread_.deleteLater();
  QThread::msleep(10);
}
void XpiderSocketThread::StartConnection(QString &host_name, int host_port){
  host_name_ = host_name;
  host_port_ = host_port;

  time_clock_.start();
  //timer_retry_.start(INTERVAL_RETRY);

  is_alive_=false;
  last_alive_tiggered_=0;

  event_thread_.start();
}

void XpiderSocketThread::SendMessage(QByteArray &raw_message){
  if(raw_message.size()){
    hdlc_.frameDecode(raw_message,raw_message.size());
  }
}

void XpiderSocketThread::onTimeoutRetry(){
  if(state()==UnconnectedState){
    qDebug()<<tr("[%1,%2]connecting to %3:%4").arg(__FILE__).arg(__LINE__).arg(host_name_).arg(host_port_);
    connectToHost(host_name_,host_port_);
  }else if(state()==ConnectedState){
    QByteArray payload;
    payload=QString("    ").toUtf8();
    payload.insert(0,XPIDER_MESSAGE_HEAD);
    write(payload);
  }
}


void XpiderSocketThread::onConnected(){
  static uint64_t  rand_seed_counter=0;
  QByteArray tx_pack;
  uint8_t* tx_buffer;
  uint16_t tx_length;
  XpiderInfo info;
  XpiderProtocol  protocol;
  protocol.Initialize(&info);

  //step1.set target angle & transform to tx buffer
  ++rand_seed_counter;
  srand(time(NULL)+rand_seed_counter);
  uint8_t r = rand()%128;
  uint8_t g = rand()%180;
  uint8_t b = rand()%255;
  info.left_led_rgb[0]=r;
  info.left_led_rgb[1]=g;
  info.left_led_rgb[2]=b;
  info.right_led_rgb[0]=r;
  info.right_led_rgb[1]=g;
  info.right_led_rgb[2]=b;

  qDebug()<<tr("[%1,%2]xpider %3:%4 connected with RGB(%5,%6,%7)")
            .arg(__FILE__).arg(__LINE__)
            .arg(host_name_)
            .arg(host_port_)
            .arg(r).arg(g).arg(b);

  protocol.GetBuffer(protocol.kFrontLeds, &tx_buffer, &tx_length);
  tx_pack.append((char*)tx_buffer,tx_length);
  SendMessage(tx_pack);
}

void XpiderSocketThread::onDisconnected(){
  qDebug()<<tr("[%1,%2]xpider %3:%4 disconnected.").arg(__FILE__).arg(__LINE__).arg(host_name_).arg(host_port_);
}

void XpiderSocketThread::onReadyRead(){
  rx_data_.clear();
  rx_data_ = read(RX_MAX_SIZE);
  if(rx_data_.size()==0)return;

  //qDebug()<<tr("[%1,%2]rx bytes:%3").arg(__FILE__).arg(__LINE__).arg(rx_data_.size());
  hdlc_.charReceiver(rx_data_);
}

void XpiderSocketThread::onHdlcDecodedByte(QByteArray decoded_data, quint16 decoded_size){
  static const int HB_TIMEOUT=5000;//10sec

  int current_time = time_clock_.elapsed();
  bool state = ((current_time-last_alive_tiggered_)<HB_TIMEOUT);
  last_alive_tiggered_ = current_time;
  is_alive_ = state;
}
void XpiderSocketThread::onHdlcEncodedByte(QByteArray encoded_data){
  QByteArray tx_payload;
  tx_payload.append(XPIDER_MESSAGE_HEAD);
  tx_payload.append(encoded_data);
  write(tx_payload);
}
bool XpiderSocketThread::Available() const{
  //return ((state()==ConnectedState) && is_alive_);
  return (state()==ConnectedState);

}

void XpiderSocketThread::StopWalking()
{
  QByteArray tx_pack;
  uint8_t* tx_buffer;
  uint16_t tx_length;
  XpiderInfo info;
  XpiderProtocol  protocol;
  protocol.Initialize(&info);

  //step1.set target angle & transform to tx buffe
  info.move = 0;
  info.rotate = 0;

  protocol.GetBuffer(protocol.kMove, &tx_buffer, &tx_length);
  tx_pack.append((char*)tx_buffer,tx_length);
  SendMessage(tx_pack);
}
