
#include "tcphttp.h"
#include "QXmlStreamReader"

TCPHTTP::TCPHTTP(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	ui.textBrowser->document()->setMaximumBlockCount(1000);

	QSettings cfgIni(tPath + "/cfg.ini", QSettings::IniFormat);
	mDataNum = cfgIni.value("/DATANUM/mDataNum").toInt();
	tUrl = cfgIni.value("/Url/SOAP_WSDL").toString();
	tDevice = cfgIni.value("/DEVICE/tDevice").toString();
	timeout = cfgIni.value("/TimeOut/timeout").toInt();
	//
	mStatusList.append("WTSP_NULL");
	mStatusList.append("WTSP_LOG_IN");
	mStatusList.append("WTSP_INPUT_ERR");
	
	for (size_t i = 0; i < mDataNum; i++)
	{
		mStatusList.append(QString("WTSP_INPUT_DATA%1").arg(i + 1));
	}

	mStatusList.append("WTSP_LOG_OUT");

	mStatusList.append("FINISH");

	pCurState = &mStatusList.first();
	
	/*
	QPalette pal;
	pal = ui.textBrowser->palette();
	pal.setColor(QPalette::Base, QColor(0, 0, 255));//改变背景色
	*/
	connect(this, SIGNAL(stepSignal(QString*)), this, SLOT(stepSlot1(QString*)));

	manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

	wdgTimer = new QTimer(this); 
	connect(wdgTimer, SIGNAL(timeout()), this, SLOT(onTimer1()));	
}

TCPHTTP::~TCPHTTP()
{

}

void TCPHTTP::on_runTimer()
{

}

void TCPHTTP::onTimer1()
{
	timeout--;

	if (timeout <= 0)
	{
		curErrorCode = ERROR_CODE_TIMEOUT;
		wdgTimer->stop();
		stopSignal(QString("%1").arg(ERROR_CODE_TIMEOUT));
	
	}
}

bool TCPHTTP::xmlLoginoutDecode(QString xmlCode)
{
	QXmlStreamReader reader(xmlCode);

	reader.setNamespaceProcessing(false);

	while (!reader.atEnd())
	{
		if (reader.isStartElement())
		{
			if (reader.name() == "WTSP_LOGINOUTResult")
			{
				auto re = reader.readElementText();
				auto a = re.mid(2, 11);
				if (PASS == re.left(1) || LOGIN_TWINCE == re.mid(2,11))
				{
					return true;
				}

			}

		}
		else if (reader.isEndElement() && reader.name() == "soap:Envelope")
		{
			
		}

		reader.readNext();
	}

	return false;
}

bool TCPHTTP::xmlInputDecode(QString xmlCode, QString& reInfo)
{
	QXmlStreamReader reader(xmlCode);

	reader.setNamespaceProcessing(false);

	while (!reader.atEnd())
	{
		if (reader.isStartElement())
		{
			if (reader.name() == "WTSP_SSD_INPUTDATAResult")
			{
				reInfo = reader.readElementText();

				if (reInfo.left(1) == PASS)
				{
					return true;
				}

			}

		}
		else if (reader.isEndElement() && reader.name() == "soap:Envelope")
		{

		}

		reader.readNext();
	}

	return false;
}

bool TCPHTTP::curStateDecode(QString* pCurState, QString reMsg)
{

	if ("WTSP_LOG_IN" == *pCurState || "WTSP_LOG_OUT" == *pCurState)
	{
		if (true == xmlLoginoutDecode(reMsg))
		{
			ui.textBrowser->append("<font color = green> Success! </font>");
			return true;
		}
	}
	else
	{
		QString reInfo;
		if (true == xmlInputDecode(reMsg, reInfo))
		{
			ui.textBrowser->append("<font color = green> Success! </font>");
			return true;
		}
		else
		{
			ui.textBrowser->append(reInfo);
		}
	}
	
	ui.textBrowser->append("<font color = red> Failed! </font>");
	return false;
}

void TCPHTTP::replyFinished(QNetworkReply *reply)
{
	//string.toUtf8()
	if (reply->error() == QNetworkReply::NoError)
	{
		QByteArray bytes = reply->readAll();
		qDebug() << bytes;
		QString reMsg = QString::fromUtf8(bytes);
		
		if (true == curStateDecode(pCurState, reMsg))
		{
			if (false == bProErr && "WTSP_LOG_IN" == *pCurState)
			{
				pCurState++;
			}

			pCurState++;
		}
		else
		{
			tryAgain++;
		}
		
	}
	else
	{
		QByteArray bytes = reply->readAll();
		QString string = QString::fromUtf8(bytes);

		qDebug() << "handle errors here";
		QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		//statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
		qDebug("found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
		qDebug(qPrintable(reply->errorString()));

		tryAgain++;
	}

	if (tryAgain >= 1)
	{
		wdgTimer->stop();
		
		if (curErrorCode != ERROR_CODE_TIMEOUT)
		{
			curErrorCode = ERROR_CODE_FAILED;
			stopSignal(QString("%1").arg(ERROR_CODE_FAILED)); //上传返回失败
		}
		
		pCurState = &mStatusList.first();
		tryAgain = 0;
	}
	else
	{
		stepSignal(pCurState);
	}

	if ("FINISH" == *pCurState)
	{
		wdgTimer->stop();
		curErrorCode = ERROR_CODE_PASSED;
		stopSignal(QString("%1").arg(ERROR_CODE_PASSED));
		pCurState = &mStatusList.first();
	}

	reply->deleteLater();
}

static void loginoutMsgEncode(QString device, QString tUser, QString para, QString & msg)
{
	msg.clear();

	msg += "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\"\r";
	msg += "xmlns:sfis=\"http://www.pegatroncorp.com/SFISWebService/\">\r";
	msg += "<soap:Header/>\r";
	msg += "<soap:Body>\r";
	msg += "<sfis:WTSP_LOGINOUT>\r";

	msg += "<!--Optional:-->\r";

	msg += "<sfis:programId>TSP_NLSPAD</sfis:programId>\r";			// = "TSP_NLSPAD"
	msg += "<!--Optional:-->\r";
	msg += "<sfis:programPassword>N3eaM;</sfis:programPassword>\r";
	msg += "<!--Optional:-->\r";

	msg += "<sfis:op>";
	msg += tUser;
	msg += "</sfis:op>\r";

	msg += "<!--Optional:-->\r";
	msg += "<sfis:password></sfis:password>\r";
	msg += "<!--Optional:-->\r";

	msg += "<sfis:device>";
	msg += device;
	msg += "</sfis:device>\r";      // DEVICE  由铠嘉提供

	msg += "<!--Optional:-->\r";
	msg += "<sfis:TSP>LINK</sfis:TSP>\r";

	msg += "<sfis:status>";
	msg += para;
	msg += "</sfis:status>\r";          //  1代表登录, 2代表登出

	msg += "</sfis:WTSP_LOGINOUT>\r";
	msg += "</soap:Body>\r";
	msg += "</soap:Envelope>\r";
}

static void inputdataMsgEncode(QString device, QString data, QString & msg)
{
	msg.clear();

	msg += "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\"\r";
	msg += "xmlns:sfis=\"http://www.pegatroncorp.com/SFISWebService/\">\r";
	msg += "<soap:Header/>\r";
	msg += "<soap:Body>\r";
	msg += "<sfis:WTSP_SSD_INPUTDATA>\r";
	msg += "<!--Optional:-->\r";
	msg += "<sfis:programId>TSP_NLSPAD</sfis:programId>\r";
	msg += "<!--Optional:-->\r";
	msg += "<sfis:programPassword>N3eaM;</sfis:programPassword>\r";
	msg += "<!--Optional:-->\r";

	msg += "<sfis:device>";
	msg += device;
	msg += "</sfis:device>\r";      // DEVICE  由铠嘉提供

	msg += "<!--ISN:-->\r";

	msg += "<sfis:data>";
	msg += data;				// KJ180910001JKKX4X48
	msg += "</sfis:data>\r";		//DATA ：error / 线别 / ISN等

	msg += "<!--Optional:-->\r";
	msg += "<sfis:type>1</sfis:type>\r";
	msg += "</sfis:WTSP_SSD_INPUTDATA>\r";
	msg += "</soap:Body>\r";
	msg += "</soap:Envelope>\r";
}

void TCPHTTP::loginout(WTSP_LOGINOUT inORout, QString device, QString tUser)
{
	//POST
	QNetworkRequest request;

	request.setUrl(QUrl(tUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setRawHeader("Content-Type", "application/soap+xml;setchar-utf8");

	QString upMsg;

	loginoutMsgEncode(device, tUser, QString("%1").arg(inORout), upMsg);

	QByteArray  postData;
	postData.append(upMsg);

	reply = manager->post(request, postData);

	qDebug() << reply->error();
}

void TCPHTTP::inputdata(QString device, QString data)
{
	//POST
	QNetworkRequest request;

	request.setUrl(QUrl(tUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	request.setRawHeader("Content-Type", "application/soap+xml;setchar-utf8");

	QString upMsg;

	inputdataMsgEncode(device, data, upMsg);

	QByteArray  postData;
	postData.append(upMsg);

	reply = manager->post(request, postData);

	qDebug() << reply->error();
}

void TCPHTTP::postTest()
{
	QSettings cfgIni(tPath + "/cfg.ini", QSettings::IniFormat);

	QString tUrl = cfgIni.value("/Url/SOAP_WSDL").toString();

	//POST
	QNetworkRequest request;

	request.setUrl(QUrl(tUrl));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
//	request.setRawHeader("Content-Type", "application/soap+xml;setchar-utf8");

	QString upMsg = "userkey=ff9bf77c827b4a4f86f4bd6b20ea9989";

	QByteArray  postData;
	postData.append(upMsg);

	reply = manager->post(request, postData);

	qDebug() << reply->error();
}


void TCPHTTP::tcpSlot(const QString upData)
{
	wdgTimer->stop();
	
	QStringList tList = upData.split("\n");

	if (tList.size() < mDataNum + 2)
	{
		stopSignal(QString("%1").arg(ERROR_CODE_FAILED));
		return;
	}

	tUser = tList[0];
	tErr = "err0" + tList[1].right(2);

	if ("err000" == tErr)
	{
		bProErr = false;
	}
	else
	{
		bProErr = true;
	}

	mDataList.clear();
	for (size_t i = 0; i < mDataNum; i++)
	{
		mDataList.append(tList[i + 2]);
	}
	

	if (*pCurState != mStatusList.first())
	{
		curErrorCode = ERROR_CODE_TIMEOUT;
		stopSignal(QString("%1").arg(ERROR_CODE_TIMEOUT));
	}
	else
	{	
		ui.textBrowser->append("");
		ui.textBrowser->append("");
		ui.textBrowser->append(QStringLiteral("Msg:"));
		ui.textBrowser->append("<font color = blue>" + upData + "</font>");

		pCurState++;

		//
		stepSignal(pCurState);

		//
		curErrorCode = 0;
		
		wdgTimer->start(1000); 
	}
}

void TCPHTTP::stepSlot1(QString* pCurState)
{
	QApplication::processEvents();

	if (*pCurState == "WTSP_NULL" || *pCurState == "FINISH")
	{
		return;
	}
	else if (*pCurState == "WTSP_LOG_IN")
	{
		ui.textBrowser->append("LOG_IN:" + tUser);
		loginout(LOG_IN, tDevice, tUser);
		return;
	}
	else if (*pCurState == "WTSP_INPUT_ERR")
	{
		ui.textBrowser->append("Err");
		inputdata(tDevice, tErr);
		return;
	}
	else if (*pCurState == "WTSP_LOG_OUT")
	{
		ui.textBrowser->append("LOG_OUT:");
		loginout(LOG_OUT, tDevice, tUser);
		return;
	}
	else
	{
		auto tState = *pCurState;
		ui.textBrowser->append(tState);
		int index = tState.right(1).toInt();
		inputdata(tDevice, mDataList.at(index - 1));
		return;
	}
}
