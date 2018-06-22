#ifndef TCPCLIENTSOCKET_H
#define TCPCLIENTSOCKET_H

#include <QTcpSocket>
#include <QObject>
class TcpClientSocket : public QTcpSocket
{
	Q_OBJECT                //��Ӻ�(Q_OBJECT)��Ϊ��ʵ���ź���۵�ͨ��
public:
	TcpClientSocket(QObject *parent = 0);
signals:
	void updateClients(QString, int);
	void disconnected(int);
	protected slots:
	void dataReceived();
	void slotDisconnected();
};

#endif // TCPCLIENTSOCKET_H
