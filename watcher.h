#ifndef WATCHER_H
#define WATCHER_H

#include <QObject>
#include <QtNetwork>
#include <qhash.h>
#include <qhostaddress.h>
#include <qstringview.h>
#include <qtcpsocket.h>
#include <qtmetamacros.h>
#include "socket.h"

class Watcher : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool listenning READ isListenning NOTIFY serverStateChanged)
  Q_PROPERTY(int messageSocketState READ messageSocketState NOTIFY messageSocketStateChanged)
  Q_PROPERTY(int fileSocketState READ fileSocketState NOTIFY fileSocketStateChanged)

public:
  Watcher();  

  bool isListenning();
  
  bool isMessageSocketConnected();
  bool isFileSocketConnected();

  int messageSocketState();
  int fileSocketState();

public slots:
  void listen(const QByteArray &address, quint16 port);

  void connectMessageSocket(const QByteArray &address, quint16 port, QIODevice::OpenMode mode = QIODevice::ReadWrite);
  void connectFileSocket(const QByteArray &address, quint16 port, QIODevice::OpenMode mode = QIODevice::ReadWrite);

  void sendMessage(const QByteArray &message) { socket.sendMessage(message); }
  void sendFile(const QUrl &src) { socket.sendFile(src); }

private slots:
  void newConnection();
  void socketIdentificator();

  void setMessageSocket();
  void setFileSocket();

  void confirmMessageSocket();
  void confirmFileSocket();

signals:
  void serverStateChanged(bool listenning);
  void messageSocketStateChanged(int state);
  void fileSocketStateChanged(int state);
  void receivedMessage(const QString message);
  void receivedFile(const QUrl filePath);
  void sendingFile(const QString &fileName, qint64 fileSize, qint64 sentSize);
  void receivingFile(const QString &fileName, qint64 fileSize, qint64 receivedSize);

  
private:
  QTcpServer server;
  // TODO: implement a mechanism of cleaning the queue.
  QHash<QTcpSocket*, QBuffer*> identificationQueue;
    
  QTcpSocket* messageSocket;
  QTcpSocket* fileSocket;

  Socket socket;

  void resetMessageSocket(QTcpSocket* socket, bool server = false);
  void resetFileSocket(QTcpSocket* socket, bool server = false);

  QBuffer messageSocketBuffer;
  QBuffer fileSocketBuffer;
};

#endif
