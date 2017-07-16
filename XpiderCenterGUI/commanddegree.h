#ifndef COMMANDDEGREE_H
#define COMMANDDEGREE_H

#include "commandthread.h"
#include "xpidersocketthread.h"
#include "optiserver.h"

class CommandDegree :public CommandParser
{
  Q_OBJECT
public:
  static const QString KEY;//"degree"
  static const QString ALL;//"all"
  static const QString RAND;//"rand"
  static constexpr int MIN_LEN=3;

  explicit CommandDegree(QObject *parent = 0);

  virtual bool Exec(QStringList argv);

  virtual const QString & Key() const{return KEY;}
protected:
  void SendCommand(int id, float delta_theta);
  float ComputeDelta(const xpider_opti_t& xpider ,float heading_theta);
};


#endif // COMMANDDEGREE_H
