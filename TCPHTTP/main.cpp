#include "tcpserver.h"
#include "tcphttp.h"
#include <QtWidgets/QApplication>



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	TCPHTTP w;
	w.show();

	TcpServer T;
//	T.show();

	a.connect(&T, SIGNAL(tcpSignal(const QString)), &w, SLOT(tcpSlot(const QString)));
	a.connect(&w, SIGNAL(stopSignal(const QString)), &T, SLOT(stopSlot(const QString)));

	

	return a.exec();
}
