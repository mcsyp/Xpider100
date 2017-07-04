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
#include "xpiderpool.h"
class OptiPostWork: public QObject{
  Q_OBJECT
public:
  explicit OptiPostWork(QObject* parent=NULL);
  void MoveToPostWork(std::map<uint32_t,xpider_target_point_t> &target_map,std::vector<xpider_opti_t> & info_list);

protected slots:
  void onXpiderPlannerUpdated();

private:
  TrajectoryPlanner planner_;
  xpider_target_point_t target_list_[XpiderPool::MAX_THREADS];
  xpider_opti_t xpider_list_[XpiderPool::MAX_THREADS];
  int list_len_;
};

class OptiService : public QTcpServer{
  Q_OBJECT
public:
  static constexpr int SERVER_PORT= 8000;//this server works at this port.
  static constexpr int RX_MAX_SIZE= 6000;
  static constexpr int INTERVAL_POST_TASK=3000;

  static constexpr int XPIDER_WALK_SPEED=90;
  static constexpr int XPIDER_ROTATE_SPEED=100;

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

  Q_INVOKABLE void pushTarget(unsigned int id, float x, float y);
  Q_INVOKABLE void removeTarget(unsigned int id);

signals:
  void xpiderUpdate(unsigned int id,float theta, float x,float y,bool is_real=false);
  void landmarkUpdate(unsigned int id,float x,float y);
  void xpiderPlannerUpdate();
  void xpiderClear();

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

public:
  std::map<uint32_t,xpider_target_point_t> target_map_;
};
#endif // ServerOpti_H
