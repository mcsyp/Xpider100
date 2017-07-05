#include "xpidersocketthread.h"

#define MESSAGE_HEAD "0155"
const QByteArray XpiderSocketThread::XPIDER_MESSAGE_HEAD = QByteArray::fromHex(MESSAGE_HEAD);
XpiderSocketThread::XpiderMap XpiderSocketThread::g_xpider_map_ = XpiderSocketThread::XpiderMap();
void XpiderSocketThread::Dispose(uint32_t id){
  for(auto iter = g_xpider_map_.begin();iter!=g_xpider_map_.end();++iter){
    if(iter->first==id){
      g_xpider_map_.erase(iter++);
      break;
    }
  }
}

void XpiderSocketThread::DisposeAll(){
  g_xpider_map_.clear();
}
XpiderSocketThread* XpiderSocketThread::Socket(uint32_t id){
  if(g_xpider_map_.count(id)){
    return g_xpider_map_[id];
  }else{
    return NULL;
  }
}
XpiderSocketThread* XpiderSocketThread::Create(QThread * work_thread){
  if(g_xpider_map_.size()>=MAX_THREADS)return NULL;

  XpiderSocketThread* xpider = new XpiderSocketThread();
  //init id
  xpider->my_id_ = g_xpider_map_.size();
  g_xpider_map_[xpider->my_id_] = xpider;

  //init event thread
  xpider->moveToThread(work_thread);
  work_thread->start();
  return xpider;
}

XpiderSocketThread::XpiderSocketThread(QObject* parent):QTcpSocket(parent){
  connect(&hdlc_,SIGNAL(hdlcTransmitByte(QByteArray)),this, SLOT(onHdlcEncodedByte(QByteArray)));
  connect(&hdlc_,SIGNAL(hdlcValidFrameReceived(QByteArray,quint16)),this,SLOT(onHdlcDecodedByte(QByteArray,quint16)));

  connect(&timer_retry_,SIGNAL(timeout()),this,SLOT(onTimeoutRetry()));

  connect(this,SIGNAL(connected()),this,SLOT(onConnected()));
  connect(this,SIGNAL(disconnected()),this,SLOT(onDisconnected()));
  connect(this,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
}
XpiderSocketThread::~XpiderSocketThread(){
  Dispose(my_id_);
}
void XpiderSocketThread::StartConnection(QString &host_name, int host_port){
  host_name_ = host_name;
  host_port_ = host_port;

  time_clock_.start();
  timer_retry_.start(INTERVAL_RETRY);

  is_alive_=false;
  last_alive_tiggered_=0;
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
  qDebug()<<tr("[%1,%2]xpider %3:%4 connected.").arg(__FILE__).arg(__LINE__).arg(host_name_).arg(host_port_);
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
