#ifndef COMMANDAIM_H
#define COMMANDAIM_H

#include <QObject>

#include "commandthread.h"
#include "xpidersocketthread.h"
#include "optiserver.h"

class CommandAim :public CommandParser
{
  Q_OBJECT
public:
  static const QString KEY;//"aim"
  static const QString ALL;//"all"
  static constexpr int MIN_LEN=4;

  explicit CommandAim(QObject *parent = 0);

  virtual bool Exec(QStringList argv);

  virtual const QString & Key() const{return KEY;}
protected:
  void SendCommand(int id, float delta_theta);
  float ComputeDelta(const xpider_opti_t& xpider ,float target_x,float target_y);
};

#endif // COMMANDAIM_H
