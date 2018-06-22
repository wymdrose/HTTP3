#include "tcpserver.h"
#include <QSettings>
#include <QDir>
#include <qDebug>

TcpServer::TcpServer(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{
	
	setWindowTitle(tr("TCP Server"));
	ContentListWidget = new QListWidget;
	PortLabel = new QLabel(tr("PORT:"));
	PortLineEdit = new QLineEdit;
	CreateBtn = new QPushButton(tr("Creat"));
	mainLayout = new QGridLayout(this);
	mainLayout->addWidget(ContentListWidget, 0, 0, 1, 2);
	mainLayout->addWidget(PortLabel, 1, 0);
	mainLayout->addWidget(PortLineEdit, 1, 1);
	
	mainLayout->addWidget(CreateBtn, 2, 0, 1, 2);
	
	
	QString tPath = QDir::currentPath();
	qDebug() << tPath;

//	QString tPath = qApp->applicationDirPath();
	QSettings cfgIni(tPath + "/cfg.ini", QSettings::IniFormat);

	port = cfgIni.value("/TcpServer/PORT").toInt();

	PortLineEdit->setText(QString::number(port));
	connect(CreateBtn, SIGNAL(clicked()), this, SLOT(slotCreateServer()));

	CreateBtn->clicked();
//	slotCreateServer();
}

void TcpServer::slotCreateServer()
{
	server = new Server(this, port);				//创建一个Server对象
	connect(server, SIGNAL(updateServer(QString, int)), this,
		SLOT(updateServer(QString, int)));	//(a)
	CreateBtn->setEnabled(false);
}

void TcpServer::updateServer(QString msg, int length)
{
	ContentListWidget->addItem(msg.left(length));

	if (msg.right(15) != "Enter Chat Room")
	{
		tcpSignal(msg.left(length));
	}
	
}

TcpServer::~TcpServer()
{

}

void TcpServer::stopSlot(const QString stopMsg)
{
	QTcpSocket *item = server->tcpClientSocketList.at(0);
	item->write(stopMsg.toLatin1());
}
