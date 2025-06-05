#include "socket.h"
#include <qdebug.h>
#include <qfileinfo.h>
#include <qlogging.h>
#include <qobject.h>
#include <qtcpsocket.h>

const qint64 MAX_SOCKET_WRITE_BUFFER_SIZE = 4096;
const qint64 MSWBS = MAX_SOCKET_WRITE_BUFFER_SIZE;

Socket::Socket(QTcpSocket* socket, QObject *parent) : QObject{parent} ,
                                                      messageSocket{nullptr},
                                                      fileSocket{nullptr},
                                                      sr{&mr},
                                                      sfr{&fr},
                                                      writtingMessage{false},
                                                      writtingFile{false},
                                                      msw{&mts},
                                                      fsw{&fts}
{
  mr.open(QIODevice::ReadWrite);
  fr.open(QIODevice::ReadWrite);
  //messageReader.open(QIODevice::ReadWrite);
  mts.open(QIODevice::ReadWrite);
  fts.open(QIODevice::ReadWrite);

  frd = &fr;
}

void Socket::disconnectMessageSocket() {
  if (messageSocket != nullptr && messageSocket->state() == QTcpSocket::ConnectedState)
  messageSocket->disconnectFromHost();
}

void Socket::disconnectFileSocket() {
  if (fileSocket != nullptr && fileSocket->state() == QTcpSocket::ConnectedState)
  fileSocket->disconnectFromHost();
}

void Socket::connectMessageSocket(const QHostAddress &address, quint16 port, QIODevice::OpenMode mode) {
  messageSocket->connectToHost(address, port, mode);
}

void Socket::connectFileSocket(const QHostAddress &address, quint16 port, QIODevice::OpenMode mode) {
  fileSocket->connectToHost(address, port, mode);
}

void Socket::setMessageSocket(QTcpSocket* socket) {
  if (socket == nullptr || socket == messageSocket) return;
  if (messageSocket != nullptr) {
    if (messageSocket->state() == QTcpSocket::ConnectedState)
      messageSocket->disconnectFromHost();
    messageSocket->disconnect();
    messageSocket->deleteLater();
  }
  messageSocket = socket;

  connect(messageSocket, &QTcpSocket::readyRead,
          this, &Socket::messageIn);
  connect(messageSocket, &QTcpSocket::bytesWritten,
          this, &Socket::writeMessageSocket);
  connect(messageSocket, &QTcpSocket::disconnected,
          this, &Socket::messageSocketDisconnected);

  msr = true;
  msz = sizeof(qint64);
}

void Socket::setFileSocket(QTcpSocket* socket) {
  if (socket == nullptr || socket == fileSocket) return;
  if (fileSocket != nullptr) {
    if (fileSocket->state() == QTcpSocket::ConnectedState)
      fileSocket->disconnectFromHost();
    fileSocket->disconnect();
    fileSocket->deleteLater();
  }
  fileSocket = socket;

  connect(fileSocket, &QTcpSocket::readyRead,
          this, &Socket::fileIn);
  connect(fileSocket, &QTcpSocket::bytesWritten,
          this, &Socket::writeFileSocket);
  connect(fileSocket, &QTcpSocket::disconnected,
          this, &Socket::fileSocketDisconnected);
  fsr = 0;
  fsz = sizeof(qint64);
}

void Socket::messageSocketDisconnected() {
  
}
void Socket::fileSocketDisconnected(){}

void Socket::messageIn() {
  mr.write(messageSocket->read(msz - mr.size()));
  if (mr.size() >= msz) {
    mr.seek(0);
    if (msr) {
      sr >> msz;
    } else {
      message = mr.buffer();
      msz = sizeof(qint64);
      emit receivedMessage(message);
    }
    mr.buffer().clear();
    mr.seek(0);
    msr = !msr;
  }
  if (messageSocket->bytesAvailable()) messageIn();
}

void Socket::fileIn() {
  frd->write(fileSocket->read(fsz - frd->size()));

  if (fsr == 3)
    emit receivingFile(rfileName, fsz, frd->size());
  
  if (frd->size() >= fsz) {
    frd->seek(0);
    QFileInfo inf;
    QDir dir;
    QString base, suffix;
    int fileNum(1);
    switch (fsr) {
    case 0:
      sfr >> fsz;
      fsr += 1;
      break;
    case 1:
      inf.setFile(fr.buffer());
      dir = inf.dir();
      base = inf.baseName();
      suffix = inf.completeSuffix();
      while (inf.exists())
        inf.setFile(dir.filePath(base + " ("+QString::number(fileNum++)+")" + suffix));
      inf.filePath();
      fs.setFileName(inf.filePath());
      rfileName = inf.fileName();
      fs.open(QIODevice::ReadWrite);
      fsz = sizeof(qint64);
      fsr = 2;
      break;
    case 2:
      sfr >> fsz;
      fsr += 1;
      frd = &fs;
      break;
    case 3:
      fs.close();      
      emit receivedFile(QUrl::fromLocalFile(fs.fileName()));
      fsz = sizeof(qint64);
      fsr = 0;
      frd = &fr;
      break;
    }
    if (fsr < 3) {
      fr.buffer().clear();
      fr.seek(0);
    }
  }
  if (fileSocket->bytesAvailable()) fileIn();
}

void Socket::sendMessage(const QByteArray &mess) {
  messageQueue.append(mess);
  writeMessageSocket();
}

bool Socket::nextM() {
  if (messageQueue.isEmpty()) return false;
  messageReader.setBuffer(&messageQueue[0]);
  messageReader.open(QIODevice::ReadOnly);
  msw << (qint64)messageQueue[0].length();
  return true; 
}

void Socket::writeMessageSocket() {
  if (messageSocket->bytesToWrite() > 0) return;
  if (writtingMessage) return;
  writtingMessage = true;
  
  while(mts.size() < MSWBS) {
    if (!messageReader.isOpen() || messageReader.atEnd()) {
      if (messageReader.isOpen() && messageReader.atEnd()) {        
        messageReader.close();
        messageQueue.removeFirst();
      }
      if (!nextM()) break;
    } else {
      mts.write(messageReader.read(MSWBS - mts.size()));
    }
  }

  if (mts.size() > 0) {
    messageSocket->write(mts.buffer());
    mts.buffer().clear(); mts.seek(0);
  }
  writtingMessage = false;
}

void Socket::sendFile(const QUrl &url) {
  if (url == QUrl()) return;
  fileQueue.append(url);
  writeFileSocket();
}

bool Socket::nextF() {
  if (fileQueue.isEmpty()) return false;
  fileToSend.setFileName(fileQueue[0].toLocalFile());
  if (fileToSend.open(QIODevice::ReadWrite)) {
    sfileName = fileQueue[0].fileName();
    fsw << sfileName.toLocal8Bit().size();
    fts.write(sfileName.toLocal8Bit());
    fileSize = fileToSend.size();
    fsw << (qint64)fileSize;
    sentSize = -fts.size();
    return true;
  } else return nextF();
}

void Socket::writeFileSocket() {
  if (fileSocket->bytesToWrite() > 0) return;
  if (writtingFile) return;
  writtingFile = true;

  while (fts.size() < MSWBS) {
    if (!fileToSend.isOpen() || fileToSend.atEnd()) {
      if (fileToSend.isOpen() && fileToSend.atEnd()) {
        fileToSend.close();
        fileQueue.removeFirst();
      }
      if (!nextF()) break;
    } else
      fts.write(fileToSend.read(MSWBS - fts.size()));
  }

  if (fts.size() > 0) {
    fileSocket->write(fts.buffer());
    sentSize += fts.size();
    if (sentSize > 0)
    emit sendingFile(sfileName, fileSize, sentSize);
    fts.buffer().clear(); fts.seek(0);
  }
  writtingFile = false;
}
