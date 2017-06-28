#ifndef SERVERXPIDER_H
#define SERVERXPIDER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QRunnable>
#include <QThreadPool>
#include <QList>
#include <QByteArray>
#include <QByteArrayList>

#include <stdint.h>
#include <vector>

#include "xpiderhdlcencoder.h"

class XpiderClient;
class XpiderServer : public QTcpServer
{
  Q_OBJECT
public:
  //friend class XpiderClient;
  static constexpr int SERVER_PORT= 9000;//this server works at this port.
  static constexpr int SERVER_MAX_THREADPOOL=150;

  explicit XpiderServer(QObject *parent = 0);
  virtual ~XpiderServer();

  //start the server
  int StartServer();

  //stop and reset the server
  void ResetServer();

  bool RemoveClient(XpiderClient* client);

protected:
  virtual void incomingConnection(qintptr socket);

protected:
  QThreadPool threadpool_;//a threadpool for all xpiders
  std::vector<XpiderClient*> clientlist_;//saves all clients
};

class XpiderClient : public QRunnable, public XpiderHdlcEncoderHandler{
  friend class ServerXpider;
public:
  static constexpr int RX_TIMEOUT=80;//jump if there no message for more than this msec
  static constexpr int RX_MAX_SIZE=128;
  void run();

  void Reset();

  /*purpose: Extract payload from the input message
   *input:
   * @raw_data, raw data from the client device
   *output:
   * out_payload, extracted content message from the raw_data. HDLC WRAP is already pealed off.
   *return:
   * true if the extraction is success
   */
  bool ExtractPayload(QByteArray & raw_data,QByteArray & out_payload);

  /*purpose: process the raw_payload message form the raw data
   *input:
   * @message, non-HDLC encoded message, and slo
   *output:
   * out_payload, extracted content message from the raw_data
   *return:
   * true if the extraction is success
   */
  virtual void onDecodedMessage(QByteArray &dec_message, quint16 dec_length);
  virtual void onEncodedMessage(QByteArray & enc_message);

protected:
  XpiderServer * server_;
  qintptr socket_descriptor_;

  //tx buffer
  QByteArrayList tx_queue_;
};
#endif // SERVERXPIDER_H
