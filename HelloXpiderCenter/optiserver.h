#ifndef ServerOpti_H
#define ServerOpti_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QRunnable>
#include <QThreadPool>
#include <QList>
#include <QByteArray>
#include <QByteArrayList>

#include <stdint.h>
#include <vector>

class OptiServer : public QTcpServer
{
  Q_OBJECT
public:
  static constexpr int SERVER_PORT= 8000;//this server works at this port.
  static constexpr int RX_MAX_SIZE=4096;

  explicit OptiServer(QObject *parent = 0);
  virtual ~OptiServer();

  //start the server
  int StartServer();

  //stop and reset the server
  void ResetServer();

protected slots:
  void onClientDisconnected();
  void onClientReadyRead();

  void onNewConnection();

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

  /*purpose: find if the input data is a message header
   *input:
   * @rx_buffer, the rx buffer from the client
   * @rx_len ,the length of the rx buffer
   *return:
   * returns true if success
   */
  static bool CheckMessageHead(uint8_t* rx_buffer, int rx_len);

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
  enum RxState{
    RxStateIdle=0, //no package is procesing
    RxStateProcessing=1, //processing a package
  };

  void RxProcess(uint8_t* data, int len);
  void RxProcessPayload(message_head * head, QByteArray & payload);

protected:
  QTcpSocket *client_;
  OptiServer::message_head rx_message_head_;//rx message head
  enum RxState rx_state_;

  //rx buffer & message
  QByteArray rx_payload_;//rx queue, waiting to be processed
  int rx_payload_size_;
};

#endif // ServerOpti_H
