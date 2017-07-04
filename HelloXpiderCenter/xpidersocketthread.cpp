#include "xpidersocketthread.h"

#define MESSAGE_HEAD "0155"
const QByteArray XpiderSocketThread::XPIDER_MESSAGE_HEAD = QByteArray::fromHex(MESSAGE_HEAD);
XpiderSocketThread::XpiderMap XpiderSocketThread::g_xpider_map_ = XpiderSocketThread::XpiderMap();

void XpiderSocketThread::Dispose(XpiderSocketThread *instance){
  if(instance==NULL)return;
  for(auto iter = g_xpider_map_.begin();iter!=g_xpider_map_.end();++iter){
    XpiderSocketThread * xpider = iter->second;
    if(xpider==instance){
      g_xpider_map_.erase(iter);
      xpider->work_thread_.exit();
      break;
    }
  }
}

void XpiderSocketThread::DisposeAll(){
  for(auto iter=g_xpider_map_.begin();iter!=g_xpider_map_.end();++iter){
    XpiderSocketThread * xpider = iter->second;
    if(xpider){
      xpider->work_thread_.exit();
    }
  }
  g_xpider_map_.clear();
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

  my_id_ = g_xpider_map_.size();
  g_xpider_map_[my_id_] = this;
}
XpiderSocketThread::~XpiderSocketThread(){
  Dispose(this);
  timer_retry_.stop();
}

void XpiderSocketThread::onTimeoutRetry(){

}
void XpiderSocketThread::onConnected(){

}

void XpiderSocketThread::onDisconnected(){

}

void XpiderSocketThread::onReadyRead(){

}

void XpiderSocketThread::onHdlcDecodedByte(QByteArray decoded_data, quint16 decoded_size){

}

void XpiderSocketThread::onHdlcEncodedByte(QByteArray encoded_data){

}
