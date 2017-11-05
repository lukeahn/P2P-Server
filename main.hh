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

void processRumor(QVariantMap inMap){
	QVariantMap nested=qvariant_cast<QVariantMap>(status["want"]);
	//Append to view
	textview->append("Received from:" + inMap["Origin"].toString());
	textview->append("Content:" + inMap["ChatText"].toString());
	//Change status
	//FIX IF STATEMENT, CONDITION IS ALWAYS TRUE

	int flag=0;
	for(QVariantMap::const_iterator iter = nested.begin(); iter != nested.end(); ++iter) {
		//Receiver already in the status message
			if(iter.key().compare(inMap["Origin"].toString())==0) {
				int tmp=nested[iter.key()].toInt();
				if(tmp!=inMap["SeqNo"].toInt()){
						flag=1;
						//Drop the packet
						break;
					}else{
				nested[iter.key()]=QVariant(++tmp);
				flag=1;
				oldEntry=qvariant_cast<QVariantMap>(oldMessagesCollection[inMap["Origin"].toString()]);
				oldEntry[inMap["SeqNo"].toString()]=QVariant(inMap["ChatText"].toString());
				oldMessagesCollection[inMap["Origin"].toString()]=QVariant(oldEntry);
				}
			}
		}
		//New receiver
		if(flag==0){
			nested[inMap["Origin"].toString()]=QVariant(1);
			QVariantMap newMessage;
			newMessage[inMap["SeqNo"].toString()]=QVariant(inMap["ChatText"].toString());
			oldMessagesCollection[inMap["Origin"].toString()]=QVariant(newMessage);
		}
		status["want"]=QVariant(nested);
}
private:
	QTextEdit *textview;
	QLineEdit *textline;
  	NetSocket *socket;
	quint32 counter;
	QVariantMap status;
	QVariantMap oldMessagesCollection;
	QVariantMap oldEntry;
};



#endif // P2PAPP_MAIN_HH
