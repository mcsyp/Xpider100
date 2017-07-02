#ifndef XPIDERINSTANCE_H
#define XPIDERINSTANCE_H

#include <QObject>

class XpiderInstance : public QObject
{
  Q_OBJECT
public:
  explicit XpiderInstance(QObject *parent = 0);
  void SocketStateUpdate(int state);
  void SocketAliveUpdate(bool is_alive);
signals:

public slots:
};

#endif // XPIDERINSTANCE_H
