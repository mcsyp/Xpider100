#ifndef OPTIPROTOCOL2_H
#define OPTIPROTOCOL2_H

#include <QObject>
#include <QByteArray>
#include <QByteArray>
#include <QTimer>

#include <stdint.h>
#include <vector>

class OptiProtocol : public QObject
{
  Q_OBJECT
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
    uint8_t cmdId;
    uint32_t len;//this is the length of the total pack include head size.
                     //e.g. len=HEAD_SIZE+PAYLOAD_SIZE
  }message_head;
#pragma pack()
  static constexpr int MESSAGE_HEAD_LEN = sizeof(message_head);

  explicit OptiProtocol(QObject* parent=0);

  void PushToProtocol(QByteArray & raw_data);

  void Reset();


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


signals:
  void PayloadReady(int cmdid,QByteArray& payload);

public slots:
private:
  enum RxState{
    RxStateIdle=0, //no package is procesing
    RxStateProcessing=1, //processing a package
  };
  message_head rx_message_head_;//rx message head
  enum RxState rx_state_;

  //rx buffer & message
  QByteArray rx_payload_;//rx queue, waiting to be processed
  int rx_payload_size_;
};

#endif // OPTIPROTOCOL2_H
