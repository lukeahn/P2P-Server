
#include <unistd.h>
#include <stdlib.h>
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <string> 
#include <iostream>
#include <cstdlib>
#include <sstream> 
#include "main.hh"

ChatDialog::ChatDialog()
{

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	// Small text-entry box the user can enter messages.
	// This widget normally expands only horizontally,
	// leaving extra vertical space for the textview widget.
	//
	// You might change this into a read/write QTextEdit,
	// so that the user can easily enter multi-line messages.
	textline = new QLineEdit(this);


	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);

	socket = new NetSocket();
    socket->bind();

    setWindowTitle(socket->portInfo);
	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
	udpSocket.bind(36768);
	connect(textline, SIGNAL(returnPressed()),
	this, SLOT(gotReturnPressed()));
	connect(&udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));
}

void ChatDialog::gotReturnPressed()
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	QVariantMap map;
	QByteArray datagram;
	int myPortMin1 = 32768 + (getuid() % 4096)*4;
	int myPortMax1 = myPortMin1 + 3;
	//Create a VariantMap
	map["ChatText"]=QVariant(textline->text());
	//Creates Stream
	QDataStream outStream(&datagram, QIODevice::WriteOnly);
	outStream << map;

	for (int p = myPortMin1; p <= myPortMax1; p++) {
		udpSocket.writeDatagram(datagram, QHostAddress::LocalHost, p);
		qDebug() << "FIX: send message to other peers: " << textline->text();
		qDebug() << "Host:: " << p;
		
		textview->append(socket->portInfo + " : " + map["ChatText"].toString() + " sent to: " + QString::number(p));
		// textview->append(Result);

	}
  qDebug() << map;
	// Clear the textline to get ready for the next input message.
	textline->clear();

}



void ChatDialog::processPendingDatagrams()
{
    QByteArray datagram;
		QVariantMap inMap;
    do {
        datagram.resize(udpSocket.pendingDatagramSize());
        udpSocket.readDatagram(datagram.data(), datagram.size());
				QDataStream inStream(&datagram, QIODevice::ReadOnly);
				inStream >> inMap;
				qDebug() << inMap;
    } while (udpSocket.hasPendingDatagrams());

}

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four P2Papp instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
	myPortMax = myPortMin + 3;
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
			portInfo.append(QString("Port "));
            portInfo.append(QString::number(p));
			qDebug() << "this is ID " << portInfo;

			return true;
		}
	}


	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Create a UDP network socket
	// NetSocket sock;
	// if (!sock.bind())
	// 	exit(1);

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}
