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
  static constexpr int RX_MAX_SIZE=4096;

  enum SERVER_CMDID {
    SERVER_PATTERN_REQ=0x1,
    SERVER_PATTERN_ACK=0x2,
    SERVER_STATUS_ACK=0x3,
  };
  explicit OptiService(QObject *parent = 0);
  virtual ~OptiService();

  //start the server
  int StartServer();

  //stop and reset the server
  void ResetServer();

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
