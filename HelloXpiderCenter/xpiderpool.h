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

#include "xpiderhdlcencoder.h"
#include "xpiderclient.h"

class XpiderPool : public QObject
{
  Q_OBJECT
public:
  //friend class XpiderClient;
  static constexpr int MAX_THREADS=100;

  explicit XpiderPool(QObject *parent = 0);
  virtual ~XpiderPool();

  //start the server
  void StartConnection();

  //stop and reset the server
  void Reset();

protected:
  QThreadPool threadpool_;//a threadpool for all xpiderss
};

#endif // SERVERXPIDER_H
