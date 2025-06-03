#ifndef WATCHER_H
#define WATCHER_H

#include <QObject>
#include <QtNetwork>
#include <qstringview.h>
#include <qtmetamacros.h>
#include "socket.h"

class Watcher : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
  Q_PROPERTY(bool server READ isServer NOTIFY serverChanged)

public:
  Watcher() : _connected{false}, server{false} {

    auto doConnections = [&](){
      
    };
    
    connect(&srv, &QTcpServer::newConnection, this, [&]() {
      socket = new Socket(srv.nextPendingConnection());
      connect(socket, &Socket::connectedChanged, this,[&](bool con) {
        _connected = con;
        emit connectedChanged(con);
      });
      connect(socket, &Socket::received, this, &Watcher::gotMessage);

      if (socket->connected()) {
        qDebug() << "Connected";
        _connected = true;
        emit connectedChanged(_connected);
      }
    });

    server = srv.listen(QHostAddress::Any, 8094);
    emit serverChanged(server);
    if (!server) {
      socket = new Socket;
      connect(socket, &Socket::connectedChanged, this,[&](bool con) {
        _connected = con;
        emit connectedChanged(con);
      });
      connect(socket, &Socket::received, this, &Watcher::gotMessage);
    }
  }
  
  bool connected() { return _connected; }
  bool isServer() { return server; }

public slots:
  void sendMessage(const QString &messageString, const QList<QUrl> &fileNames) {
    socket->sendMessage(messageString, fileNames);
  }

  void connectToSrv() {
    socket->doConnect("127.0.0.1", 8094);
  }

signals:
  void connectedChanged(bool connected);
  void serverChanged(bool server);

  void gotMessage(const QString &message, const QList<QByteArray> files);
  
private:
  QTcpServer srv;
  Socket* socket;
  bool server;
  bool _connected;
};

#endif
