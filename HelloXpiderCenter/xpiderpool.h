#ifndef SERVERXPIDER_H
#define SERVERXPIDER_H
#include <QTcpSocket>
#include <QRunnable>
#include <QThreadPool>
#include <QList>
#include <QByteArray>
#include <QByteArrayList>

#include <stdint.h>
#include <vector>
#include <map>

#include "xpiderhdlcencoder.h"
#include "xpidersocket.h"

class XpiderPool : public QObject
{
  Q_OBJECT
public:
  static constexpr int MAX_THREADS=120;

  explicit XpiderPool(QObject *parent = 0);
  virtual ~XpiderPool();

  //start the server
  Q_INVOKABLE void StartConnection();

  //stop and reset the server
  Q_INVOKABLE void StopConnection();


protected:
  QThreadPool threadpool_;//a threadpool for all xpiderss
};

#endif // SERVERXPIDER_H
