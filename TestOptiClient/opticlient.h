#ifndef OPTICLIENT_H
#define OPTICLIENT_H

#include <QTcpSocket>
#include <QByteArray>
#include <QTimer>
#include "optiprotocol.h"
class OptiClient: public QTcpSocket
{
  Q_OBJECT
public:

  static constexpr int TIMEOUT_RETRY=2000;
  static constexpr int TIMEOUT_MESSAGE=1000;

  OptiClient(QObject* parent=NULL);

  void TryConnectToHost(QString hostname,int port);

protected slots:
  void onReadyRead();
  void onConnected();
  void onDisconnected();

  void onRetryTimeout();
  void onMessageTimeout();

private:
  QTimer timer_retry_;
  QTimer timer_message_;
  QString host_name_;
  int host_port_;

  OptiProtocol protocol_;
};

#endif // OPTICLIENT_H
