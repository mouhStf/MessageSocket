#ifndef SOCKET_H
#define SOCKET_H

#include <QBuffer>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QTcpSocket>
#include <QUrl>
#include <QDir>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qstringview.h>
#include <qtypes.h>

class Socket : public QObject {
  Q_OBJECT

public:
  Socket(QTcpSocket* socket = nullptr, QObject *parent = nullptr);

signals:
  void receivedMessage(const QString message);
  void receivedFile(const QUrl filePath);

  void sendingFile(const QString &fileName, qint64 fileSize, qint64 sentSize);
  void receivingFile(const QString &fileName, qint64 fileSize, qint64 receivedSize);

public slots:
  void setMessageSocket(QTcpSocket* socket);
  void setFileSocket(QTcpSocket* socket);

  void disconnectMessageSocket();
  void disconnectFileSocket();
  
  void connectMessageSocket(const QHostAddress &address, quint16 port, QIODevice::OpenMode mode = QIODevice::ReadWrite);
  void connectFileSocket(const QHostAddress &address, quint16 port, QIODevice::OpenMode mode = QIODevice::ReadWrite);

  void sendMessage(const QByteArray &mess);
  void sendFile(const QUrl &url);
                                   
private slots:
  void messageIn();
  void fileIn();

  void writeMessageSocket();
  void writeFileSocket();

  void messageSocketDisconnected();
  void fileSocketDisconnected();

private:
  QTcpSocket* messageSocket;
  QTcpSocket* fileSocket;

  bool msr;
  qint64 msz;
  QBuffer mr;
  QDataStream sr;
  QByteArray message;

  int fsr;
  qint64 fsz;
  QBuffer fr;
  QFile fs;
  QIODevice *frd;
  QDataStream sfr;

  bool writtingMessage;
  QList<QByteArray> messageQueue;
  QBuffer messageReader;
  QBuffer mts;
  QDataStream msw;
  bool nextM();

  bool writtingFile;
  QList<QUrl> fileQueue;
  QFile fileToSend;
  QBuffer fts;
  QDataStream fsw;
  bool nextF();

  QString rfileName;
  QString sfileName;
  qint64 fileSize;
  qint64 sentSize;
};

#endif
