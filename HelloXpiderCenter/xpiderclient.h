#ifndef XPIDERCLIENT_H
#define XPIDERCLIENT_H
#include <QTime>
#include <QRunnable>
#include <QThreadPool>
#include <QList>
#include <QByteArray>
#include <QByteArrayList>
#include <QTcpSocket>

#include <stdint.h>
#include <vector>

#include "xpiderhdlcencoder.h"

class XpiderClient : public QRunnable, public XpiderHdlcEncoderHandler{
protected:
  XpiderClient();
public:
  typedef std::vector<XpiderClient*> XpiderList;
  static XpiderList g_xpider_list_;


  static const QByteArray XPIDER_MESSAGE_HEAD;
  static constexpr int XPIDER_MESSAGE_LEN=2;

  static constexpr int RETRY_INTERVAL=2000;
  static constexpr int HB_INTERVAL=5000;//send heart beat info to xpider every 3sec
  static constexpr int RX_TIMEOUT=100;//jump if there no message for more than this msec
  static constexpr int RX_MAX_SIZE=128;

  static XpiderClient* Create(QString host_name, int port);
  static void RemoveInstance(XpiderClient* instance);
  static void DisposeAllClients();//dispose all xpider clients.

  /*Run the xclient process in a seperate thread*/
  void run();

  /*Reset the client*/
  void Reset();

  /*Purpose:append the message to tx_queue and send them later
   *input:
   * @tx_message, non-hdlc wrapped and non-0155 head included
   */
  void AppendTxMessage(QByteArray & tx_message);

  virtual void onDecodedMessage(QByteArray &dec_message, quint16 dec_length);
  virtual void onEncodedMessage(QByteArray & enc_message);

  void XpiderConnectedAction();
protected:
  void ConnectionRetry(int current_msec,QTcpSocket & socket);
  void ConnectionHeartBeat(int current_msec, QTcpSocket & socket);

protected:
  //tx buffer
  QByteArrayList tx_queue_;
  QString host_name_;
  int host_port_;
  bool is_running_;

private:
  QTcpSocket * ptr_socket_;
  XpiderHdlcEncoder * ptr_hdlc_;
  QTime * ptr_time_;
  bool is_alive_;
};


#endif // XPIDERCLIENT_H
