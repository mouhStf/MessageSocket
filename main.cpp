#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtNetwork>
#include <qhostaddress.h>
#include <qlogging.h>
#include <qtcpserver.h>

#include "socket.h"

int main(int argc, char *argv[]) {
  
  QGuiApplication app(argc, argv);

  QTcpServer srv;
  if (srv.listen(QHostAddress::Any, 8094))
    qDebug() << "Listening on " << 8094;
  Socket socket;

  QObject::connect(&srv, &QTcpServer::newConnection, [&]() {
    qDebug() << "New connection";
    socket.setSocket(srv.nextPendingConnection());
  });

  QQmlApplicationEngine engine;

  engine.rootContext()->setContextProperty("socket", &socket);

  QObject::connect(&engine,&QQmlApplicationEngine::objectCreationFailed,
                   &app,[]() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

  engine.loadFromModule("MessageSocket", "Main");

  return app.exec();
}
