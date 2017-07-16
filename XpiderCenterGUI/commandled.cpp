#include "commandled.h"
#include <QTextStream>
#include <map>
#include <xpider_ctl/xpider_info.h>
#include <xpider_ctl/xpider_protocol.h>
#include <xpidersocketthread.h>

const QString CommandLed::KEY = QString("led");
const QString CommandLed::ALL = QString("all");
const QString CommandLed::LED_L = QString("-l");
const QString CommandLed::LED_R = QString("-r");
const QString CommandLed::LED_BOTH = QString("-b");
const QString CommandLed::LED_RANDOM = QString("rand");

CommandLed::CommandLed(QObject* parent):CommandParser(parent){
  QTextStream text;
  text.setString(&example_);
  text<<"led [index] -l [r] [g] [b] -r [r] [g] [b]"<<endl;
  text<<"eg: led 10 -l 255 128 190 -r 230 154 31"<<endl;
  text<<"eg: led 10 -b 255 128 190"<<endl;
  text<<"eg: led all -b 255 128 190"<<endl;
}

bool CommandLed::Exec(QStringList argv){
  if(argv.length()==0 ||
     argv.size()<MIN_LEN ||
     argv[0].isEmpty()){
    return false;
  }
  //step1 check first key
  if(argv[0]!=KEY)return false;

  //step2 check index
  int id=0;
  bool is_all_included=false;
  if(argv[1]==ALL){
    is_all_included=true;
  }else{
    id = argv[1].toInt();
  }

  //step3. parse the color
  uint8_t left[]={0,0,0};
  uint8_t right[]={0,0,0};

  int offset_left = argv.indexOf(LED_L);
  int offset_right = argv.indexOf(LED_R);
  int offset_both = argv.indexOf(LED_BOTH);

  if(offset_both>0 && argv.length()>=offset_both+4){
    //if "-b" is set, both led are the same color
    left[0] = argv[offset_both+1].toInt();
    left[1] = argv[offset_both+2].toInt();
    left[2] = argv[offset_both+3].toInt();
    right[0] = argv[offset_both+1].toInt();
    right[1] = argv[offset_both+2].toInt();
    right[2] = argv[offset_both+3].toInt();
  }else{
    if(offset_left>0 && argv.length()>=offset_left+4){
      left[0] = argv[offset_left+1].toInt();
      left[1] = argv[offset_left+2].toInt();
      left[2] = argv[offset_left+3].toInt();
    }
    if(offset_right>0 && argv.length()>=offset_right+4){
      right[0] = argv[offset_right+1].toInt();
      right[1] = argv[offset_right+2].toInt();
      right[2] = argv[offset_right+3].toInt();
    }
  }

  //step4. executate
  do{
    for(int i=0;i<XpiderSocketThread::socket_list_.size();++i){
      if(is_all_included || i==id){
        XpiderSocketThread* x = XpiderSocketThread::socket_list_.at(i);
        if(argv.contains(LED_RANDOM)){
          static unsigned int rand_counter=0;
          ++rand_counter;
          srand(rand_counter);
          for(int i=0;i<3;++i){
            uint8_t color = rand()%255;
            left[i] = color;
            right[i] = color;
          }
        }
        QByteArray tx_pack;
        uint8_t* tx_buffer;
        uint16_t tx_length;
        XpiderInfo info;
        XpiderProtocol  protocol;
        protocol.Initialize(&info);

        //step1.set target angle & transform to tx buffer
        info.left_led_rgb[0]=left[0];
        info.left_led_rgb[1]=left[1];
        info.left_led_rgb[2]=left[2];
        info.right_led_rgb[0]=right[0];
        info.right_led_rgb[1]=right[1];
        info.right_led_rgb[2]=right[2];
        qDebug()<<tr("[%1,%2] %3,%4,%5,%6,%7,%8").arg(__FILE__).arg(__LINE__).arg(left[0]).arg(left[1]).arg(left[2]).arg(right[0]).arg(right[1]).arg(right[2]);

        protocol.GetBuffer(protocol.kFrontLeds, &tx_buffer, &tx_length);
        tx_pack.append((char*)tx_buffer,tx_length);

        x->SendMessage(tx_pack);
      }
    }
  }while(0);

  return true;
}
