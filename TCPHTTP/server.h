#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QObject>
#include "tcpclientsocket.h"    //����TCP���׽���
class Server : public QTcpServer
{
	Q_OBJECT                    //��Ӻ�(Q_OBJECT)��Ϊ��ʵ���ź���۵�ͨ��
public:
	Server(QObject *parent = 0, int port = 0);
	QList<TcpClientSocket*> tcpClientSocketList;
signals:
	void updateServer(QString, int);
	public slots:
	void updateClients(QString, int);
	void slotDisconnected(int);
protected:
	void Server::incomingConnection(qintptr);
//	void incomingConnection(int socketDescriptor);
};

#endif // SERVER_H
