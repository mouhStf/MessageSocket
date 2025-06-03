#include "socket.h"

const qint64 MAX_SOCKET_WRITE_BUFFER_SIZE = 4096;
const qint64 MSWBS = MAX_SOCKET_WRITE_BUFFER_SIZE;

Socket::Socket(QTcpSocket* socket, QObject *parent) : QObject{parent} , socket{nullptr}, _connected{false} {  
  if (socket == nullptr)
    setSocket(new QTcpSocket);
  else
    setSocket(socket);
}

void Socket::setSocket(QTcpSocket* _socket) {
  if (this->socket != nullptr) {
    if (this->socket->state() == QTcpSocket::ConnectedState)
      this->socket->disconnectFromHost();

    this->socket->disconnect();
    this->socket->deleteLater();
  }

  this->socket = _socket;

  connect(socket, &QTcpSocket::readyRead, this, &Socket::inF);
  connect(socket, &QTcpSocket::bytesWritten, this, &Socket::bh);

  if (socket->state() == QTcpSocket::ConnectedState) {
    _connected = true;
    emit connectedChanged(_connected);
  }

  connect(socket, &QTcpSocket::stateChanged, this, [&](QTcpSocket::SocketState st) {
    if (st == QTcpSocket::ConnectedState) {
      qDebug() << "Connected";
      _connected = true;
      emit connectedChanged(_connected);
    }
  });

  connect(socket, &QTcpSocket::disconnected, this, [&]() {
    qDebug() << "Disconnected";
    _connected = false;
    emit connectedChanged(_connected);
  });

  idx = 0;
  gi = false;
  fm = false;  
  _size = sizeof(qint64);
    bufw.open(QIODevice::WriteOnly);
  _sttw.setDevice(&bufw);

  hb = new QBuffer;
  hb->open(QIODevice::ReadWrite);
  szr.setDevice(hb);
  fb = new QFile;

  srd = hb;
}

void Socket::inF() {
  srd->write(socket->read(_size - srd->size()));

  if (srd->size() >= _size) {
    srd->seek(0);
    switch (idx) {
    case 0:
      szr >> _size;
      hb->buffer().clear();
      hb->seek(0);

      fm = !fns.empty();
      if (fm) {
        files.append(fns.takeFirst());
        fb->setFileName(files.last());
        qDebug() << "Opened file" << fb->open(QIODevice::WriteOnly);
        srd = fb;
      } 
      idx = 1;

      break;
    case 1:
      if (fm) {
        qDebug() << "Done writing file" << fb->fileName();
        fb->close();
      } else {
        QList<QByteArray> ps = hb->buffer().split(0x1F);
        hb->buffer().clear();
        hb->seek(0);
        message = ps.takeFirst();
        fns = ps;
        qDebug() << "Got message:" << message;
        qDebug() << "file list" << fns;
      }
      if (fns.empty()) {
        emit received(message, files);
        files.clear();
      }

      if (srd != hb) srd = hb;      
      _size = sizeof(qint64);
      idx = 0;
      break;
    }
  }

  if (socket->bytesAvailable()) inF();
}

void Socket::doConnect(QString addr, int port) {
  qDebug() << "Connecting to" << addr << port;
  socket->connectToHost(addr, port);
}

void Socket::sendMessage(const QString &messageString, const QList<QUrl> &fileNames) {
  QPair<QString, QList<QUrl>> el(messageString, fileNames);
  queue.append(el);
  bh();
}

void Socket::bh() {
  if (socket->bytesToWrite() > 0) return;
  
  if (!gi) {
    bufq.clear();
    bidx = -1;

    if (queue.isEmpty()) return;

    QSharedPointer<QIODevice> bfs(new QBuffer);

    bfs->open(QIODevice::ReadWrite);
    bfs->write(queue[0].first.toLocal8Bit());

    bufq.append(bfs);
    
    for (const QUrl &url : queue[0].second) {
      bfs->write(0x1F + url.fileName().toLocal8Bit());
      QSharedPointer<QIODevice> f(new QFile(url.toLocalFile()));
      if (f->open(QIODevice::ReadOnly))
        bufq.append(f);
      else
        qDebug() << "Could not open" << url;
    }
    bfs->seek(0);
    gi = true;
  }

  gh();
}

void Socket::gh() {

  while (bufw.size() < MSWBS) {
    if (bidx == -1 || bufq[bidx]->atEnd()) {
      if (bidx > -1) bufq[idx]->close();
      if (++bidx < bufq.size()) {
        _sttw << (qint64)bufq[bidx]->size();
      } else {
        queue.removeAt(0);
        gi = false;
        break;
      }
    } else {
      bufw.write(bufq[bidx]->read(MSWBS - bufw.size()));
    }
  }

  if (bufw.size() > 0) {
    socket->write(bufw.buffer());
    bufw.buffer().clear();
    bufw.seek(0);
  }
}
