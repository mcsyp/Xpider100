#include "commandthread.h"
#include <QStringList>
#include <QTextStream>

#include "commanddelay.h"
#include "commandaim.h"
#include "commandled.h"
#include "commanddegree.h"
#include "commandwalk.h"
#include "commandrotate.h"

//QMap<int, CommandThread*> CommandThread::g_cmd_map = QMap<int, CommandThread*>();
CommandThread::CommandThread(QObject *parent) : QThread(parent)
{
  //init command list
  //led
  CommandLed *cmdled=new CommandLed;
  cmd_map_.insert(cmdled->Key(),cmdled);

  //delay
  CommandDelay * cmd_delay = new CommandDelay;
  cmd_map_.insert(cmd_delay->Key(),cmd_delay);

  //aim
  CommandAim * cmd_aim = new CommandAim;
  cmd_map_.insert(cmd_aim->Key(),cmd_aim);

  //degree
  CommandDegree* cmd_degree = new CommandDegree;
  cmd_map_.insert(cmd_degree->Key(),cmd_degree);

  //walk
  CommandWalk* cmd_walk = new CommandWalk;
  cmd_map_.insert(cmd_walk->Key(),cmd_walk);

  //rotate
  CommandRotate * cmd_rotate = new CommandRotate;
  cmd_map_.insert(cmd_rotate->Key(),cmd_rotate);

  //clear the cmd string
  cmd_text_.clear();
}

CommandThread::~CommandThread()
{
  for(auto iter=cmd_map_.begin();iter!=cmd_map_.end();++iter){
    CommandParser* ptr = iter.value();
    if(ptr){
      delete ptr;
    }
  }
  cmd_map_.clear();
}

void CommandThread::StartCommandChain(QString &command_text){
  if(command_text.isEmpty())return;
  cmd_text_ =  command_text;
  qDebug()<<tr("[%1,%2] cmd:%3").arg(__FILE__).arg(__LINE__).arg(command_text);
  start();//run the thread
}

void CommandThread::run()
{
  if(cmd_text_.isEmpty())return;

  QTextStream stream;
  stream.setString(&cmd_text_);

  //qDebug()<<tr("[%1,%2]cmd_string=%3").arg(__FILE__).arg(__LINE__).arg(cmd_text_);
  while(!stream.atEnd()){
    QString cmd_line = stream.readLine();
    QStringList cmd_arg_list = cmd_line.split(" ");
    if(cmd_arg_list.size()<CMD_MIN_LEN){
      continue;
    }
    //step1 get the key
    QString key = cmd_arg_list[0];
    if(key.isEmpty())continue;

#if 0
    for(int i=0;i<cmd_arg_list.length();++i){
      qDebug()<<tr("[%1,%2]").arg(__FILE__).arg(__LINE__)<<cmd_arg_list[i];
    }
#endif

    //step2. exec the command
    CommandParser * parser = cmd_map_.value(key);
    if(parser &&  parser->Exec(cmd_arg_list)){
      qDebug()<<tr("[%1,%2] [%3] successfully done.").arg(__FILE__).arg(__LINE__).arg(cmd_line);
    }else{
      qDebug()<<tr("[%1,%2] Fail to exec [%3]").arg(__FILE__).arg(__LINE__).arg(cmd_line);
    }
  }

}
