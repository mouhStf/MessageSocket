#ifndef SOCKET_H
#define SOCKET_H

#include <QBuffer>
#include <QFile>
#include <QJsonDocument>
#include <QTemporaryFile>
#include <QTcpSocket>
#include <QUrl>
#include <qstringview.h>

class Socket : public QObject {
  Q_OBJECT

public:
  Socket(QTcpSocket* socket = nullptr, QObject *parent = nullptr);

  bool connected() { return _connected; }

signals:
  void connectedChanged(bool connected);

public slots:
  void doConnect(QString addr, int port);
  void setSocket(QTcpSocket* socket);
  void sendMessage(const QString &messageString, const QList<QUrl> &fileNames);
                                   
private slots:
  void inF();
  void bh();

signals:
  void received(QString message, QList<QByteArray> files);

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
  
  QBuffer bufw;
  QDataStream _sttw;
  
  QTcpSocket* socket;

  QList<QPair<QString, QList<QUrl>>> queue;

  bool gi;
  void gh();
  QList<QSharedPointer<QIODevice>> bufq ;
  int bidx;

  bool _connected;
};

#endif
