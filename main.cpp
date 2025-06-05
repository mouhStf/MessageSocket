#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "watcher.h"

int main(int argc, char *argv[]) {
  
  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;

  Watcher watcher;
  
  qmlRegisterType<Watcher>("imps", 1, 0, "Watcher");

  QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,
                   &app,[]() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("MessageSocket", "Main");

  return app.exec();
}
