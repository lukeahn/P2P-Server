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

class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	QString portInfo;
	QString origin;
	NetSocket();
	quint32 port;
	int myPortMin = 32768 + (getuid() % 4096)*4;
	int myPortMax = myPortMin + 3;



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

private:
	QTextEdit *textview;
	QLineEdit *textline;
	QUdpSocket udpSocket;
    NetSocket *socket;
		quint32 counter;

};



#endif // P2PAPP_MAIN_HH
