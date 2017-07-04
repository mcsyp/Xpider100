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
  OptiService opti;


  QQmlApplicationEngine engine;

  QQmlContext * ctx = engine.rootContext();
  ctx->setContextProperty("opti_server_",&opti);
  ctx->setContextProperty("xpider_center_",&pool);

  engine.load(QUrl(QLatin1String("qrc:/main.qml")));

  opti.StartServer();
  pool.StartConnection();

  return app.exec();
}
