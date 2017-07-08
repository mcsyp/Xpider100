#ifndef COMMANDLED_H
#define COMMANDLED_H
#include "commandthread.h"
#include "xpidersocketthread.h"

class CommandLed :public CommandParser
{
public:
  static const QString LED_KEY;//"led"
  static const QString LED_ALL;//"all"
  static const QString LED_L;//"-l"
  static const QString LED_R;//"-r"
  static const QString LED_BOTH;//"-b"
  static constexpr int LED_MIN_LEN=6;

  CommandLed(QObject * parent=NULL);

  virtual bool Exec(QStringList argv);

  virtual const QString & Example() const;
  virtual const QString & Key() const{return LED_KEY;}
protected:
  QString example_;
};

#endif // COMMANDLED_H
