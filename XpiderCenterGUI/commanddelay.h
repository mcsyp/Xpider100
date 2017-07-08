#ifndef COMMANDDELAY_H
#define COMMANDDELAY_H

#include "commandthread.h"
#include "xpidersocketthread.h"

class CommandDelay :public CommandParser
{
  Q_OBJECT
public:
  static const QString DELAY_KEY;//"delay"
  static constexpr int DELAY_MIN_LEN=2;
  static constexpr int DELAY_MIN=100;//100ms
  static constexpr int DELAY_MAX=60000;//1 min

  explicit CommandDelay(QObject *parent = 0);

  virtual bool Exec(QStringList argv);

  virtual const QString & Example() const{return example_;}
  virtual const QString & Key() const{return DELAY_KEY;}

protected:
  QString example_;
};

#endif // COMMANDDELAY_H
