#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "optiserver.h"
#include <QDir>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  OptiService *ptr_service = OptiService::Singleton();
  QQmlApplicationEngine engine;

  QQmlContext * ctx = engine.rootContext();
  QString path = QDir::currentPath();
  qDebug()<<path;
  ctx->setContextProperty("opti_server_",ptr_service);
  engine.load(QUrl(QLatin1String("qrc:/main.qml")));

  ptr_service->StartService();

  return app.exec();
}
