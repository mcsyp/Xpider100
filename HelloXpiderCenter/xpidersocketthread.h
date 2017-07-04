#ifndef XPIDERSOCKETTHREAD_H
#define XPIDERSOCKETTHREAD_H

#include <QThread>
#include <QTimer>
#include <QTime>
#include <QTcpSocket>
#include <QByteArray>

#include <stdint.h>
#include <vector>

#include <hdlc_qt.h>

class XpiderSocketThread:public QTcpSocket
{
  Q_OBJECT
public:
  typedef std::map<int, XpiderSocketThread*> XpiderMap;
  static const QByteArray XPIDER_MESSAGE_HEAD;
  static constexpr int XPIDER_MESSAGE_LEN=2;

  static constexpr int INTERVAL_RETRY=2000;
  static constexpr int INTERVAL_TXHB=5000;//send heart beat info to xpider every 3sec
  static constexpr int RX_MAX_SIZE=128;

  static int  SocketCount();
  static void Dispose(XpiderSocketThread* thread);
  static void DisposeAll();
  static XpiderSocketThread* Socket(uint32_t id);

  XpiderSocketThread(QObject* parent=NULL);
  virtual ~XpiderSocketThread();

  void TryConnection(QString &host_name,int host_port);
  void StopConnection();

  /*Purpose:append the message to tx_queue and send them later
   *input:
   * @tx_message, non-hdlc wrapped and non-0155 head included
   */
  void SendMessage(QByteArray& raw_message);

public slots:
  void onTimeoutRetry();
  void onConnected();
  void onDisconnected();
  void onReadyRead();

  void onHdlcDecodedByte(QByteArray decoded_data, quint16 decoded_size);
  void onHdlcEncodedByte(QByteArray encoded_data);
public:
  static XpiderMap g_xpider_map_;

protected:
  QThread work_thread_;
  QTimer timer_retry_;
  QTime time_clock_;

  QString host_name_;
  int host_port_;
  uint32_t my_id_;

  hdlc_qt::HDLC_qt hdlc_;
};

#endif // XPIDERSOCKETTHREAD_H
