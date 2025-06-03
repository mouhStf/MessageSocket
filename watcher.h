#ifndef WATCHER_H
#define WATCHER_H

#include <QObject>
#include <QtNetwork>
#include <qhostaddress.h>
#include <qstringview.h>
#include <qtmetamacros.h>
#include "socket.h"

class Watcher : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
  Q_PROPERTY(bool server READ isServer NOTIFY serverChanged)

public:
  Watcher() : _connected{false}, server{false} {
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
  }
  
  bool connected() { return _connected; }
  bool isServer() { return server; }

public slots:
  void serv(int port) {
    server = srv.listen(QHostAddress::Any, port);
    emit serverChanged(server);
  }
  void sendMessage(const QString &messageString, const QList<QUrl> &fileNames) {
    socket->sendMessage(messageString, fileNames);
  }
  void connectToSrv(const QString host, int port) {    
    if (!server) {
      socket = new Socket;
      connect(socket, &Socket::connectedChanged, this,[&](bool con) {
        _connected = con;
        emit connectedChanged(con);
      });
      connect(socket, &Socket::received, this, &Watcher::gotMessage);
          
      socket->doConnect(host, port);
    } else
      qDebug() << "Cannot connect while serving";
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
