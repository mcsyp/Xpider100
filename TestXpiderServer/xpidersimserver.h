#ifndef XPIDERSIMSERVER_H
#define XPIDERSIMSERVER_H

#include <QTcpServer>
#include <QTcpSocket>

class XpiderSimServer : public QTcpServer
{
  Q_OBJECT
public:
  XpiderSimServer(QObject* parent=NULL);

protected slots:
  void onNewConnection();

  void onClientReadyRead();
  void onClientDisconnected();

protected:
  QTcpSocket* ptr_soket_;
};


#endif // XPIDERSIMSERVER_H
