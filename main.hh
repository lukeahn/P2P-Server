#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>


class NetSocket : public QUdpSocket
{
	Q_OBJECT

public:
	QString portInfo;
	QString origin; 
	NetSocket();


	// Bind this socket to a P2Papp-specific default port.
	bool bind();

private:
	int myPortMin, myPortMax;
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

};



#endif // P2PAPP_MAIN_HH

