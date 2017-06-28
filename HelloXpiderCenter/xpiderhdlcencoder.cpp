#include "xpiderhdlcencoder.h"

XpiderHdlcEncoder::XpiderHdlcEncoder(XpiderHdlcEncoderHandler *handler, QObject *parent):QObject(parent)
{
  connect(&hdlc_,SIGNAL(hdlcTransmitByte(QByteArray)),this, SLOT(onHdlcTransmitByte(QByteArray)));
  connect(&hdlc_,SIGNAL(hdlcValidFrameReceived(QByteArray,quint16)),this,SLOT(onHdlcValidFrameReceived(QByteArray,quint16)));
  handler_ = handler;
}

void XpiderHdlcEncoder::onHdlcTransmitByte(QByteArray encoded_data){
  if(handler_){ handler_->onEncodedMessage(encoded_data); }
}

void XpiderHdlcEncoder::onHdlcValidFrameReceived(QByteArray decoded_data, quint16 frame_size){
  if(handler_){ handler_->onDecodedMessage(decoded_data,frame_size); }
}
