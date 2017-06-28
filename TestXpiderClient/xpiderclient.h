#ifndef XPIDERCLIENT_H
#define XPIDERCLIENT_H
#include <QTcpSocket>
#include <QTimer>
#include "hdlc_qt.h"

class XpiderClient : public QObject
{
  Q_OBJECT
public:
  static constexpr int CLIENT_RETRY_INTERVAL = 3000;//retry every 3 sec
  static constexpr int CLIENT_MESSAGE_INTERVAL = 1000;//retry every 3 sec
  static constexpr int CLIENT_MESSAGE_SIZE = 64;//retry every 3 sec
  static constexpr int SERVER_PORT= 9000;//this server works at this port.

  explicit XpiderClient(QObject *parent = 0);

  void ConnectToHost(QString address, int port);

protected:
  void TestCase0();
  void TestCase1();

public slots:
  void onReadyRead();
  void onConnected();
  void onDisconnected();
  void onRetryTimeout();
  void onHBTimer();

  void onHdlcValidFrameReceived(QByteArray decoded_data, quint16 decoded_size);
  void onHdlcTransmitByte(QByteArray encoded_data);

  void onTimeoutSingle();
protected:
  QTcpSocket socket_;
  QTimer timer_retry_;
  QTimer timer_hb_;

  QString host_address_;
  int host_port_;
  hdlc_qt::HDLC_qt hdlc_encoder_;
};


#endif // XPIDERCLIENT_H
