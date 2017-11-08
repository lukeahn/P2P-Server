#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <unistd.h>
#include <stdlib.h>
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <string>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <QTimer>

class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	QString portInfo;
	QString origin;
	NetSocket();


	quint32 port;
	int myPortMin = 32768 + (getuid() % 4096)*4;
	int myPortMax = myPortMin + 2;



	// Bind this socket to a P2Papp-specific default port.
	bool bind();

private:


};



class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog();

public slots:
	void gotReturnPressed();
	void processPendingDatagrams();
	void processRumor(QVariantMap inMap,quint16 port);
	void processStatus(QMap<QString, QVariant> neighborMap, quint16 port);
	void sendRumor(QString myOrigin,QString mySeqNo, quint16 myPort);
	void sendStatus(quint16 myPort);

private:


	QTextEdit *textview;
	QLineEdit *textline;
  	NetSocket *socket;
		QTimer *timer;
	quint32 counter;
	QVariantMap status;
	QVariantMap oldMessagesCollection;
	QVariantMap oldEntry;
};



#endif // P2PAPP_MAIN_HH
