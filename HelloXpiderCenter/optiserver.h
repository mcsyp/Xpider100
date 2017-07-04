#ifndef SERVEROPTI_H
#define SERVEROPTI_H
#include <QByteArray>
#include <QTcpServer>
#include <QThread>
#include <QTime>

#include <stdint.h>
#include <vector>

#include "optiprotocol.h"
#include "trajectoryplanner.h"
#include "xpider_location.h"
class OptiPostWork: public QObject{
  Q_OBJECT
public:
  explicit OptiPostWork(QObject* parent=NULL);

protected slots:
  void onXpiderPlannerUpdated();

private:
  //TrajectoryPlanner planner_;
};

class OptiService : public QTcpServer{
  Q_OBJECT
public:
  static constexpr int SERVER_PORT= 8000;//this server works at this port.
  static constexpr int RX_MAX_SIZE= 80;
  static constexpr int INTERVAL_POST_TASK=5000;

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
  void xpiderUpdate(unsigned int id,float theta, float x,float y);
  void xpiderPlannerUpdate();

protected slots:
  void onClientDisconnected();
  void onClientReadyRead();

  void onNewConnection();

  void onPayloadReady(int cmdid,QByteArray & payload);
private:
  QTcpSocket *client_;
  OptiProtocol protocol_;
  QTime time_;
  int last_trigger_;
  QThread worker_thread_;
  OptiPostWork post_worker_;
  XpiderLocation *xpider_location_;
};
#endif // ServerOpti_H
