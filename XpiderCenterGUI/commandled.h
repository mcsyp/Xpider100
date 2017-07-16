#ifndef COMMANDLED_H
#define COMMANDLED_H
#include "commandthread.h"
#include "xpidersocketthread.h"

class CommandLed :public CommandParser
{
public:
  static const QString KEY;//"led"
  static const QString ALL;//"all"
  static constexpr int MIN_LEN=3;
  static const QString LED_L;//"-l"
  static const QString LED_R;//"-r"
  static const QString LED_BOTH;//"-b"
  static const QString LED_RANDOM;//"rand"

  CommandLed(QObject * parent=NULL);

  virtual bool Exec(QStringList argv);

  virtual const QString & Key() const{return KEY;}
protected:
  QString example_;
};

#endif // COMMANDLED_H
