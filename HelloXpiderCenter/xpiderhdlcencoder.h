#ifndef XPIDERHDLCENCODER_H
#define XPIDERHDLCENCODER_H
#include <QObject>
#include  "hdlc_qt.h"

class XpiderHdlcEncoderHandler{
public:
  virtual void onDecodedMessage(QByteArray & decoded_message, quint16 encoded_length)=0;
  virtual void onEncodedMessage(QByteArray & encoded_message)=0;
};

class XpiderHdlcEncoder: public QObject{
  Q_OBJECT
public:
  XpiderHdlcEncoder(XpiderHdlcEncoderHandler* handler, QObject* parent=NULL);

protected slots:
  void onHdlcValidFrameReceived(QByteArray decoded_data, quint16 decoded_size);
  void onHdlcTransmitByte(QByteArray encoded_data);

private:
  XpiderHdlcEncoderHandler * handler_;

public:
  hdlc_qt::HDLC_qt hdlc_;
};
#endif // XPIDERHDLCENCODER_H
