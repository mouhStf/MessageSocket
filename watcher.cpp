#include "watcher.h"
#include <qdebug.h>
#include <qhostaddress.h>
#include <qlogging.h>
#include <qstringview.h>
#include <qtcpsocket.h>

const qint64 IS = 4;
const QByteArray MESSI = QByteArray::fromHex("1F2F3F4F");
const QByteArray FILSI = QByteArray::fromHex("1E2E3E4E");
const QByteArray CONFIRMMESS = QByteArray::fromHex("1E2E3E4E");
const QByteArray CONFIRMFIL = QByteArray::fromHex("1E2E3E4E");

Watcher::Watcher() : messageSocket{nullptr}, fileSocket{nullptr} {
  connect(&server, &QTcpServer::newConnection,
          this, &Watcher::newConnection);

  messageSocketBuffer.open(QIODevice::ReadWrite);
  fileSocketBuffer.open(QIODevice::ReadWrite);
  
  resetMessageSocket(new QTcpSocket);
  resetFileSocket(new QTcpSocket);

  connect(&socket, &Socket::receivedMessage,
          this, &Watcher::receivedMessage);
  connect(&socket, &Socket::receivedFile,
          this, &Watcher::receivedFile);
  connect(&socket, &Socket::sendingFile,
          this, &Watcher::sendingFile);
  connect(&socket, &Socket::receivingFile,
          this, &Watcher::receivingFile);

}

void Watcher::resetMessageSocket(QTcpSocket* socket, bool server) {
  if (this->messageSocket != nullptr) {
    if (messageSocket->state() == QTcpSocket::ConnectedState)
      messageSocket->disconnectFromHost();
    messageSocket->disconnect();
    messageSocket->deleteLater();
  }
  messageSocket = socket;
  if (!server)
    connect(messageSocket, &QTcpSocket::connected,
            this, &Watcher::setMessageSocket);  
  connect(messageSocket, &QTcpSocket::stateChanged,
          this, &Watcher::messageSocketStateChanged);
  emit messageSocketStateChanged(messageSocket->state());
}

void Watcher::resetFileSocket(QTcpSocket* socket, bool server) {
  if (this->fileSocket != nullptr) {
    if (fileSocket->state() == QTcpSocket::ConnectedState)
      fileSocket->disconnectFromHost();
    fileSocket->disconnect();
    fileSocket->deleteLater();
  }
  fileSocket = socket;
  if (!server)
    connect(fileSocket, &QTcpSocket::connected,
            this, &Watcher::setFileSocket);
  connect(fileSocket, &QTcpSocket::stateChanged,
          this, &Watcher::fileSocketStateChanged);
  emit fileSocketStateChanged(fileSocket->state());
}

bool Watcher::isListenning() {
  return server.isListening();
}

void Watcher::listen(const QByteArray &address, quint16 port) {
  if (!server.isListening())
    emit serverStateChanged( server.listen(QHostAddress(address), port) );
}

void Watcher::newConnection() {
  QTcpSocket* socket = server.nextPendingConnection();
  identificationQueue[socket] = new QBuffer;
  identificationQueue[socket]->open(QIODevice::ReadWrite);
  connect(socket, &QTcpSocket::readyRead,
          this, &Watcher::socketIdentificator);
  if (socket->bytesAvailable()) emit socket->readyRead();
}

void Watcher::socketIdentificator() {
  QTcpSocket* socket = (QTcpSocket*)sender();
  
  identificationQueue[socket]->write(socket->read(IS - identificationQueue[socket]->size()));
  if (identificationQueue[socket]->size() >= IS) {
    disconnect(socket, &QTcpSocket::readyRead,
               this, &Watcher::socketIdentificator);
    identificationQueue[socket]->seek(0);    
    if (identificationQueue[socket]->buffer() == MESSI) {
      socket->write(CONFIRMMESS);
      resetMessageSocket(socket, true);
      this->socket.setMessageSocket(messageSocket);
    } else if (identificationQueue[socket]->buffer() == FILSI) {
      socket->write(CONFIRMFIL);
      resetFileSocket(socket, true);
      this->socket.setFileSocket(fileSocket);
    } else 
      identificationQueue[socket]->deleteLater();
    identificationQueue.remove(socket);
  }
  if (socket->bytesAvailable()) emit socket->readyRead();
}

void Watcher::connectMessageSocket(const QByteArray &address, quint16 port, QIODevice::OpenMode mode) {
  messageSocket->connectToHost(QHostAddress(address), port, mode);
}
void Watcher::connectFileSocket(const QByteArray &address, quint16 port, QIODevice::OpenMode mode) {
  fileSocket->connectToHost(QHostAddress(address), port, mode);
}

void Watcher::setMessageSocket() {  
  connect(messageSocket, &QTcpSocket::readyRead,
          this, &Watcher::confirmMessageSocket);
  messageSocket->write(MESSI);  
}

void Watcher::confirmMessageSocket() {
  messageSocketBuffer.write(messageSocket->readAll());

  if (messageSocketBuffer.size() >= CONFIRMMESS.size()) {
    disconnect(messageSocket, &QTcpSocket::readyRead,
             this, &Watcher::confirmMessageSocket);
    if (messageSocketBuffer.buffer() == CONFIRMMESS) {
      socket.setMessageSocket(messageSocket);      
    } else {
      messageSocketBuffer.buffer().clear();
      messageSocketBuffer.seek(0);
    }
  } else if (messageSocket->bytesAvailable()) confirmMessageSocket();
}

void Watcher::setFileSocket() {
  connect(fileSocket, &QTcpSocket::readyRead,
          this, &Watcher::confirmFileSocket);
  fileSocket->write(FILSI);
}

void Watcher::confirmFileSocket() {
  fileSocketBuffer.write(fileSocket->readAll());
  if (fileSocketBuffer.size() >= CONFIRMFIL.size()) {
    disconnect(fileSocket, &QTcpSocket::readyRead,
               this, &Watcher::confirmFileSocket);
    if (fileSocketBuffer.buffer() == CONFIRMFIL) {
      socket.setFileSocket(fileSocket);
    } else {
      fileSocketBuffer.buffer().clear();
      fileSocketBuffer.seek(0);
    }
  } else if (fileSocket->bytesAvailable()) confirmFileSocket();
}

int Watcher::messageSocketState() {
  return messageSocket->state();
}
int Watcher::fileSocketState() {
  return fileSocket->state();
}
