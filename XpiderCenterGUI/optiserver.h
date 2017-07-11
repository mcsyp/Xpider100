#ifndef SERVEROPTI_H
#define SERVEROPTI_H
#include <QByteArray>
#include <QTcpServer>
#include <QThread>
#include <QTime>

#include <stdint.h>
#include <vector>
#include <QPointF>
#include <QMap>

#include "optiprotocol.h"
#include "trajectoryplanner.h"
#include "xpider_location.h"
#include "xpidersocketthread.h"

#include "commandthread.h"
#include "trajectorythread.h"

class OptiService : public QTcpServer{
  Q_OBJECT
protected:
  explicit OptiService(QObject *parent = 0);
public:
  static constexpr int SERVER_PORT= 8000;//this server works at this port.
  static constexpr int RX_MAX_SIZE= 6000;
  static constexpr int INTERVAL_POST_TASK=2000;

  static constexpr int XPIDER_WALK_SPEED=85;
  static constexpr int XPIDER_ROTATE_SPEED=100;

  enum SERVER_CMDID {
    SERVER_UPLAOD_REQ=0x9,
    SERVER_UPLAOD_ACK=0xA,
  };
  static OptiService* Singleton();
  virtual ~OptiService();

  //start the server
  int StartService();

  //push and remove target
  Q_INVOKABLE void pushTarget(unsigned int id, float x, float y);
  Q_INVOKABLE void removeTarget(unsigned int id);
  Q_INVOKABLE void clearTargets();

  //enable the trajectory planner thread
  Q_INVOKABLE void enablePlanner(bool b);

  //run the command list in command thread framework
  Q_INVOKABLE void runCommandText(QString cmd_text);

  Q_INVOKABLE bool csvLoadTargets(QString path);
  Q_INVOKABLE bool csvSaveTargets(QString path);
signals:
  void serviceInitializing();

  void landmarkListUpdate(QString str_json);
  void xpiderListUpdate(QString str_json);
protected:
  static OptiService* ptr_instance;
protected slots:
  void onClientDisconnected();
  void onClientReadyRead();

  void onNewConnection();

  void onPayloadReady(int cmdid,QByteArray & payload);
private:
  /*purpose: this will sync the xpider opti pos and target pos by the sync flag in target strcuture
   *input:
   * @xpider_list: the vector saves location-processed xpider opti, with available ID in it.
   */
  void SyncXpiderTarget(std::vector<xpider_opti_t> & xpider_list);

  /*purpose: process the XpiderSocketThread list to find out those availible xpider's index
   *input:
   * @id_size, the max size of the xpider id_array
   *output:
   * @id_array, those available xpider socket's ID are saved in this array
   *return:
   * length of the output id_array
   */
  int AvailableXpiderSocketID(uint32_t id_array[],int id_size);

  //convert a Point class into a QString
  QString PointToString(const QPointF& point);
private:
  //opti socket related part
  QTcpSocket *client_;
  OptiProtocol protocol_;

  //time relatedd task
  QTime time_;
  QTimer timer_retry_;
  int last_trigger_;

  //xpider location service
  XpiderLocation *ptr_location_;

  bool is_planner_running_;
  TrajectoryThread *ptr_planner_thread_;

  //command frameworkrelated
  CommandThread *ptr_cmd_thread_;

  QMap<unsigned int,QPointF> ui_target_mask_;//if ui updated any target info, save its flag in this mask
};
#endif // ServerOpti_H
