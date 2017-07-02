#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "../HelloXpiderCenter/xpiderpool.h"
#include "../HelloXpiderCenter/optiserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  XpiderPool pool;
  pool.StartConnection();

  OptiService opti_service;
  opti_service.StartServer();

  QQmlApplicationEngine engine;
  engine.load(QUrl(QLatin1String("qrc:/main.qml")));

  engine.rootContext()->setContextProperty("xpider_opti_",&opti_service);
  engine.rootContext()->setContextProperty("xpider_center_",&pool);

  return app.exec();
}
