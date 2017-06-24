#ifndef SERVERXPIDER_H
#define SERVERXPIDER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QRunnable>
#include <QThreadPool>

class ClientXpider;
class ServerXpider : public QTcpServer
{
  Q_OBJECT
public:
  //magic number
  static constexpr int  MAGIC_NUM1=0x4a;
  static constexpr int  MAGIC_NUM2=0x44;
  static constexpr int  MAGIC_NUM3=0x43;
  static constexpr int  MAGIC_NUM4=0x43;

#pragma pack (1)
  typedef struct message_head_s{
    uint8_t magic_num1;
    uint8_t magic_num2;
    uint8_t magic_num3;
    uint8_t magic_num4;
    uint16_t cmdId;
    uint32_t len;//this is the length of the total pack include head size.
                     //e.g. len=HEAD_SIZE+PAYLOAD_SIZE
  }message_head;
#pragma pack()
  static constexpr int MESSAGE_HEAD_LEN = sizeof(message_head);

  enum SERVER_CMDID {
    SERVER_TIME_REQ=0x1,
    SERVER_TIME_ACK=0x2,
    SERVER_FILE_REQ=0x3,
    SERVER_FILE_ACK=0x4,
    SERVER_HEART_REQ=0x5,
    SERVER_HEART_ACK=0x6,
    SERVER_RECORD_REQ=0x7,
    SERVER_RECORD_ACK=0x8,
    SERVER_BATTERY_REQ=0x9,
    SERVER_LOG_REQ=0x0B,
  };

  static constexpr int XPIDER_PORT= 8000;//this server works at this port.
  static constexpr int MAX_THREADPOOL=150;

  explicit ServerXpider(QObject *parent = 0);
  virtual ~ServerXpider();

  int StartServer();//start the server
  void ResetServer();//stop and reset the server

  /*purpose: find the message head from the rx_buffer
   *input:
   * @rx_buffer, the rx buffer from the client
   * @rx_len ,the length of the rx buffer
   *return:
   * the offset of the beging of the head.
   * return -1 if nothing found
   */
  int FindMessageHead(uint8_t * rx_buffer, int rx_len);

protected:
  virtual void incomingConnection(qintptr socket);

  /*purpose:fill the message head of the buffer
   *input:
   * @buffer, the buffer to send out
   * @buffer_len, the length of the buffer
   */
  void FillHead(uint8_t * buffer, int buffer_size);

protected:
  QThreadPool threadpool_;//a threadpool for all xpiders
};

class ClientXpider : public QRunnable{
  friend class ServerXpider;
public:
  static constexpr int RX_MAX_SIZE=128;
  void run();

protected:
  qintptr socketDescriptor_;
};

#endif // SERVERXPIDER_H
