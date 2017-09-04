#ifndef COMMANDROTATE_H
#define COMMANDROTATE_H

#include "commandthread.h"
#include "xpidersocketthread.h"
#include "optiserver.h"

class CommandRotate :public CommandParser
{
  Q_OBJECT
public:
  static const QString KEY;//"rotate"
  static const QString ALL;//"all"
  static constexpr int MIN_LEN=3;
  static constexpr int MIN_ROTATE_SPEED=0;
  static constexpr int MAX_ROTATE_SPEED=85;

  explicit CommandRotate(QObject *parent = 0);

  virtual bool Exec(QStringList argv);

  virtual const QString & Key() const{return KEY;}
protected:
  void SendCommand(int id, int speed);
};

#endif // COMMANDROTATE_H
