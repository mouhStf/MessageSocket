#ifndef SOCKET_H
#define SOCKET_H

#include <QBuffer>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QTcpSocket>
#include <QUrl>

class Socket : public QObject {
  Q_OBJECT

public:
  Socket(QTcpSocket* socket = nullptr, QObject *parent = nullptr);

public slots:
  void doConnect(QString addr, int port);

  void setSocket(QTcpSocket* socket);

  void sendMessage(const QString &messageString, const QList<QUrl> &fileNames);
                                   
private slots:
  void inF();
  void bh(); 

private:

  QDataStream szr;
  QIODevice* srd;
  QBuffer* hb;
  QFile* fb;
  qint64 _size;
  int idx;

  QByteArray message;
  QList<QByteArray> files;
  QList<QByteArray> fns;
  bool fm;
  
  void initBuffers();
  void resetBuffer();

  QBuffer bufw;
  QDataStream _sttw;
  
  QTcpSocket* socket;

  QList<QPair<QString, QList<QUrl>>> queue;

  bool gi;
  void gh();
  QList<QSharedPointer<QIODevice>> bufq ;
  int bidx;
};

#endif
