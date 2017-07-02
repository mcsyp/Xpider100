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
#include "xpiderinstance.h"

class XpiderSocket : public QRunnable, public XpiderHdlcEncoderHandler{
protected:
  XpiderSocket();
public:
  static const QByteArray XPIDER_MESSAGE_HEAD;
  static constexpr int XPIDER_MESSAGE_LEN=2;

  static constexpr int RETRY_INTERVAL=2000;
  static constexpr int HB_INTERVAL=5000;//send heart beat info to xpider every 3sec
  static constexpr int RX_TIMEOUT=100;//jump if there no message for more than this msec
  static constexpr int RX_MAX_SIZE=128;

  static XpiderSocket* Create(QString host_name, int port, XpiderInstance * xpider);
  static void Dispose(XpiderSocket* instance);
  static void DisposeAll();//dispose all xpider clients.
  static XpiderSocket* Search(uint32_t id);//search for xpider instance by id

  virtual ~XpiderSocket();

  /*Run the xclient process in a seperate thread*/
  void run();

  /*Reset the client*/
  void Reset();



  /*xpider thread info message call back*/
  virtual void onDecodedMessage(QByteArray &dec_message, quint16 dec_length);
  virtual void onEncodedMessage(QByteArray & enc_message);

  /*some customized actions for testing*/
  void TestingAction();

  /*Purpose:append the message to tx_queue and send them later
   *input:
   * @tx_message, non-hdlc wrapped and non-0155 head included
   */
  void AppendTxMessage(QByteArray & tx_message);
  uint32_t XpdierID() const{return xpider_id_;}

public:
  typedef std::map<uint32_t, XpiderSocket*> XpiderMap;
  static XpiderMap g_xpider_map_;
protected:
  void ConnectionRetry(int current_msec,QTcpSocket & socket);
  void ConnectionTxHeartBeat(int current_msec, QTcpSocket & socket);

protected:
  //tx buffer
  QByteArrayList tx_queue_;
  QString host_name_;
  int host_port_;
  uint32_t xpider_id_;
  bool is_running_;

private:
  QTcpSocket * ptr_socket_;
  XpiderHdlcEncoder * ptr_hdlc_;
  QTime * ptr_time_;
  XpiderInstance * xpider_event_;
};


#endif // XPIDERCLIENT_H
