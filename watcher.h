#ifndef WATCHER_H
#define WATCHER_H

#include <QObject>
#include <QtNetwork>
#include "socket.h"

class Watcher : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool listenning READ isListenning NOTIFY serverStateChanged)
  Q_PROPERTY(int messageSocketState READ messageSocketState
             NOTIFY messageSocketStateChanged)
  Q_PROPERTY(int fileSocketState READ fileSocketState
             NOTIFY fileSocketStateChanged)

public:
  Watcher(QObject* parent = nullptr);

  bool isListenning();
  
  bool isMessageSocketConnected();
  bool isFileSocketConnected();

  QTcpSocket* getMessageSocket() {return messageSocket; }
  int messageSocketState();
  int fileSocketState();

  quint16 serverPort() const;
  QHostAddress messageSocketPeerAddress() const;
  quint16 messageSocketPeerPort() const;
  QString messageSocketPeerIdentifcation() const;
  QHostAddress fileSocketPeerAddress() const;
  quint16 fileSocketPeerPort() const;
  QString fileSocketPeerIdentifcation() const;

public slots:
  void listen(const QByteArray &address, quint16 port);
  void closeServer();

  void connectMessageSocket(const QHostAddress &address, quint16 port,
                            QIODevice::OpenMode mode = QIODevice::ReadWrite);
  
  void connectMessageSocket(const QByteArray &address, quint16 port,
                            QIODevice::OpenMode mode = QIODevice::ReadWrite);
  
  void connectFileSocket(const QHostAddress &address, quint16 port,
                         QIODevice::OpenMode mode = QIODevice::ReadWrite);
  
  void connectFileSocket(const QByteArray &address, quint16 port,
                         QIODevice::OpenMode mode = QIODevice::ReadWrite);

  void sendMessage(const QByteArray &message);
  void sendFile(const QUrl &src);

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
  void receivedMessage(const QByteArray message);
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
