#ifndef COMMANDWALK_H
#define COMMANDWALK_H

#include "commandthread.h"
#include "xpidersocketthread.h"
#include "optiserver.h"

class CommandWalk :public CommandParser
{
  Q_OBJECT
public:
  static const QString KEY;//"walk"
  static const QString ALL;//"all"
  static constexpr int MIN_LEN=3;

  explicit CommandWalk(QObject *parent = 0);

  virtual bool Exec(QStringList argv);

  virtual const QString & Key() const{return KEY;}
protected:
  void SendCommand(int id, int step);
};

#endif // COMMANDWALK_H
