#ifndef XPIDERCLIENT_H
#define XPIDERCLIENT_H
#include <QTcpSocket>
#include <QTimer>

class XpiderClient : public QObject
{
  Q_OBJECT
public:
  static constexpr int CLIENT_RETRY_INTERVAL = 3000;//retry every 3 sec
  static constexpr int CLIENT_MESSAGE_INTERVAL = 1000;//retry every 3 sec
  static constexpr int CLIENT_MESSAGE_SIZE = 64;//retry every 3 sec
  static constexpr int SERVER_PORT= 8000;//this server works at this port.

  explicit XpiderClient(QObject *parent = 0);

  void ConnectToHost(QString address, int port);

public:
  /**************************************
   **SERVER PROTOCOL RELATED**
   **************************************/
  //magic number
  static constexpr int  MAGIC_NUM1=0x4a;
  static constexpr int  MAGIC_NUM2=0x44;
  static constexpr int  MAGIC_NUM3=0x43;
  static constexpr int  MAGIC_NUM4=0x43;
  static constexpr int  MAGIC_NUM_LEN=4;
  //magic struct design
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
    SERVER_WALK_REQ=0x1,
    SERVER_WALK_ACK=0x2,
    SERVER_ROTATE_REQ=0x3,
    SERVER_ROTATE_ACK=0x4,
    SERVER_HEART_REQ=0x5,
    SERVER_HEART_ACK=0x6,
    SERVER_BATTERY_REQ=0x7,
    SERVER_BATTERY_ACK=0x8,
    SERVER_LOG_REQ=0x09,
    SERVER_LOG_ACK=0x0A,
  };
  /*purpose:fill the message head of the buffer
   *input:
   * @cmdid, the command id
   * @payload_len, the length of payload, HEAD size not included
   * @buffer, the buffer to send out
   * @buffer_len, the length of the buffer
   *return:
   * length of header
   * returns 0 if failed
   */
  static int FillHead(uint16_t cmdid, uint16_t payload_len, uint8_t * buffer, int buffer_size);

protected:
  void SendMessage(int cmdid, char* buffer, int buffer_len);

  void TestCase0();
public slots:
  void onReadyRead();
  void onConnected();
  void onDisconnected();
  void onRetryTimeout();

protected:
  QTcpSocket socket_;
  QTimer timer_retry_;

  QString host_address_;
  int host_port_;
};


#endif // XPIDERCLIENT_H
