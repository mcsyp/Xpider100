#ifndef SERVEROPTI_H
#define SERVEROPTI_H
#include <QByteArray>
#include <QTcpServer>

#include <stdint.h>
#include <vector>

#include "optiprotocol.h"

class OptiService : public QTcpServer{
  Q_OBJECT
public:
  static constexpr int SERVER_PORT= 8000;//this server works at this port.
  static constexpr int RX_MAX_SIZE= 4096;

  enum SERVER_CMDID {
    SERVER_UPLAOD_REQ=0x9,
    SERVER_UPLAOD_ACK=0xA,
  };
  explicit OptiService(QObject *parent = 0);
  virtual ~OptiService();

  //start the server
  int StartServer();

  //stop and reset the server
  void StopServer();

signals:
  Q_INVOKABLE void xpiderUpdate(int index,float theta, float x,float y);

protected slots:
  void onClientDisconnected();
  void onClientReadyRead();

  void onNewConnection();

  void onPayloadReady(int cmdid,QByteArray & payload);
private:
  QTcpSocket *client_;
  OptiProtocol protocol_;

};
#endif // ServerOpti_H
