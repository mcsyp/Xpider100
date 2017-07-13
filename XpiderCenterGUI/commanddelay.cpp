#include "commanddelay.h"

const QString CommandDelay::KEY = QString("delay");
CommandDelay::CommandDelay(QObject *parent) : CommandParser(parent)
{
  QTextStream text;
  text.setString(&example_);
  text<<"delay [msec]"<<endl;
  text<<"eg: delay 200"<<endl;
}

bool CommandDelay::Exec(QStringList argv){
  if(argv.length()==0 ||
     argv.size()<MIN_LEN ||
     argv[0].isEmpty()){
    return false;
  }
  //step1 check first key
  if(argv[0]!=KEY)return false;

  //step2 get the msec
  int msec = argv[1].toInt();
  qDebug()<<tr("[%1,%2] delay %3 ms\n").arg(__FILE__).arg(__LINE__);
  if(msec<DELAY_MIN || msec>DELAY_MAX){
    return false;
  }

  QThread::msleep(msec);

  return true;
}
