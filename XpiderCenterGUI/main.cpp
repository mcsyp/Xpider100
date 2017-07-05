#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "optiserver.h"

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  OptiService opti;

  QQmlApplicationEngine engine;

  QQmlContext * ctx = engine.rootContext();
  ctx->setContextProperty("opti_server_",&opti);
  engine.load(QUrl(QLatin1String("qrc:/main.qml")));

  opti.StartService();

  return app.exec();
}
