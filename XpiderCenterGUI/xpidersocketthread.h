#ifndef XPIDERSOCKETTHREAD_H
#define XPIDERSOCKETTHREAD_H

#include <QThread>
#include <QTimer>
#include <QTime>
#include <QTcpSocket>
#include <QByteArray>
#include <QVector>

#include <stdint.h>
#include <vector>

#include "global_xpider.h"

#include <hdlc_qt.h>

class XpiderSocketThread:public QTcpSocket
{
  Q_OBJECT
public:
  typedef QVector<XpiderSocketThread*> XpiderList;
  static const QByteArray XPIDER_MESSAGE_HEAD;
  static constexpr int XPIDER_MESSAGE_LEN=2;

  static constexpr int XPIDER_RETRY_TIMEOUT=2000;
  static constexpr int RX_HB_TIMEOUT=1500;//10sec
  static constexpr int RX_HB_MAX=100000;
  static constexpr int TX_HB_TIMEOUT=5000;

  static XpiderList socket_list_;

  XpiderSocketThread(QObject* parent=NULL);
  virtual ~XpiderSocketThread();

  void StartConnection(QString &host_name,int host_port);
  /*Purpose:append the message to tx_queue and send them later
   *input:
   * @tx_message, non-hdlc wrapped and non-0155 head included
   */
  void SendMessage(QByteArray& raw_message);

  bool Available() const;

  void StopWalking();
  QString Hostname()const{ return host_name_;}
  int HbCounter()const{return hb_counter_;}
signals:
  void aliveStateChange(bool alive,const XpiderSocketThread* socket);
public slots:
  void onTimeoutRetry();

  void onConnected();
  void onDisconnected();
  void onReadyRead();

  void onHdlcDecodedByte(QByteArray decoded_data, quint16 decoded_size);
  void onHdlcEncodedByte(QByteArray encoded_data);
protected:
  //QTimer timer_retry_;
  QTime hb_time_;

  QString host_name_;
  int host_port_;

  hdlc_qt::HDLC_qt hdlc_;
  QThread event_thread_;

  int hb_counter_;

  QByteArray rx_data_;
public:
  bool ui_selected_;
};

#endif // XPIDERSOCKETTHREAD_H
