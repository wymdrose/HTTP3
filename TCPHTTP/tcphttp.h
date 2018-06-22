#ifndef TCPHTTP_H
#define TCPHTTP_H

#include <QtWidgets/QMainWindow>
#include "ui_tcphttp.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

#include <QSettings>
#include <QTimer>

class TCPHTTP : public QMainWindow
{
	Q_OBJECT

public:
	TCPHTTP(QWidget *parent = 0);
	~TCPHTTP();


signals:
	void stopSignal(const QString);
	void stepSignal(QString*);

private slots:
	void replyFinished(QNetworkReply *);
	void tcpSlot(const QString);
	void stepSlot1(QString*);
	void onTimer1();
	void on_runTimer();

public:
	const QString tPath = qApp->applicationDirPath();
	const QString PASS = "1";
	const QString LOGIN_TWINCE = "Login Twice";
	const int ERROR_CODE_PASSED = 1;
	const int ERROR_CODE_FAILED = 89;
	const int ERROR_CODE_TIMEOUT = 99;

public:
	enum WTSP_LOGINOUT
	{
		LOG_IN = 1,
		LOG_OUT = 2
	};

	void loginout(WTSP_LOGINOUT inORout, QString device, QString tUser);
	void inputdata(QString device, QString data);
	void postTest();
	bool xmlLoginoutDecode(QString xmlCode);
	bool xmlInputDecode(QString xmlCode, QString& reInfo);

	bool TCPHTTP::curStateDecode(QString* pCurState, QString reMsg);

private:
	QString tUrl;
	QString tDevice;
	bool	bProErr = false;
	QString tUser;
	QString tErr;

	QStringList mStatusList;
	QStringList mDataList;
	int	mDataNum;
	
	QString* pCurState;
	int tryAgain = 0;
	QMutex m_mutex;

	QTimer *wdgTimer;
	int timeout;
	int curErrorCode = 0;

private:
	QNetworkAccessManager *manager;
	QNetworkReply *reply;

	Ui::TCPHTTPClass ui;
};

#endif // TCPHTTP_H
