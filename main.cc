#include "main.hh"


ChatDialog::ChatDialog()
{

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);
	counter=0;
	// Small text-entry box the user can enter messages.
	// This widget normally expands only horizontally,
	// leaving extra vertical space for the textview widget.
	//
	// You might change this into a read/write QTextEdit,
	// so that the user can easily enter multi-line messages.
	textline = new QLineEdit(this);
	QVariantMap nested;
	nested["initialize"]=QVariant(0);
	status["Want"]=QVariantMap(nested);

	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);

	socket = new NetSocket();
    if (!socket->bind())
        exit(1);
    qDebug() << "Bound with (from) main" <<socket->port;
    // qDebug() << socket.myPortMin <<"bingo";

    setWindowTitle(socket->portInfo);
	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
	// udpSocket.bind(36768);


	connect(textline, SIGNAL(returnPressed()),
	this, SLOT(gotReturnPressed()));

	connect(socket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));


}

void ChatDialog::gotReturnPressed()
{
	// Initially, just echo the string locally.
	// Insert some networking code here...
	QVariantMap map;
	QByteArray datagram;
	qint64 value=0;

	int myPortMin1 = 32768 + (getuid() % 4096)*4;
	int myPortMax1 = myPortMin1 + 3;
	//Create a VariantMap
	// map["ChatText"]=QVariant(textline->text());
	// map["Origin"]=QVariant(socket->port);
	// map["SeqNo"]=QVariant(counter++);

	// map["Want"]=QVariant(counter++);


	//Creates Stream
	QDataStream outStream(&datagram, QIODevice::WriteOnly);
	outStream << map;

	for (int p = myPortMin1; p <= myPortMax1; p++) {
        value=socket->writeDatagram(datagram, QHostAddress("127.0.0.1"), p);


		textview->append(socket->portInfo + " : " + map["ChatText"].toString() + " sent to: " + QString::number(p));

	}
	textview->append(map["ChatText"].toString());


	// Clear the textline to get ready for the next input message.
	textline->clear();

}



void ChatDialog::processPendingDatagrams()

{
  QByteArray datagram;
	QVariantMap inMap;
	QHostAddress address;
	quint16 port;
	QVariantMap nested=qvariant_cast<QVariantMap>(status["Want"]);
    do {
		//Receive the datagram
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size(),&address, &port);

		QDataStream inStream(&datagram, QIODevice::ReadOnly);
		inStream >> inMap;


		//check if is a status(want) or rumor
		if (inMap.contains("Want")) {
			QMap<QString, QVariant> neighborMap = inMap["Want"].toMap();
            processStatus(neighborMap, port);
        } else {
        	 processRumor(inMap,port);
        	 QVariantMap nested=qvariant_cast<QVariantMap>(status["Want"]);
        	 qDebug() << nested;
        }

		//Append to view
		textview->append("Received from:" + inMap["Origin"].toString());
		textview->append("Content:" + inMap["ChatText"].toString());
		//Change status
		//FIX IF STATEMENT, CONDITION IS ALWAYS TRUE

		} while (socket->hasPendingDatagrams());
}


void ChatDialog::processRumor(QVariantMap inMap, quint16 port)

{
	QVariantMap nested=qvariant_cast<QVariantMap>(status["Want"]);
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
		status["Want"]=QVariant(nested);
		sendStatus(port);
}

void ChatDialog::processStatus(QMap<QString, QVariant> neighborMap , quint16 port) {
	// qDebug() << "ProcessStatus"<< neighborMap;
	QVariantMap nested=qvariant_cast<QVariantMap>(status["Want"]);

    // check my values
    for (QVariantMap::const_iterator iter = nested.begin(); iter != nested.end(); ++iter) {
        QString Origin = iter.key();
        quint32 seqNo =  iter.value().toUInt();

        // case1- I need to share info
        if (!neighborMap.contains(Origin) || neighborMap[Origin].toUInt() < seqNo) {

            quint32 indexToSend = 0;
            if (!neighborMap.contains(Origin)) {
                indexToSend = 0;
            } else {
                indexToSend = neighborMap[Origin].toUInt();
            }
						QString index= QVariant(indexToSend).toString();
            //send origin and indexToSend
						sendRumor(Origin,index, port);
            return;

        }
    }
    // check neighbor's Map
    for (QVariantMap::const_iterator iter = neighborMap.begin(); iter != neighborMap.end(); ++iter) {
        QString neighborOrigin = iter.key();
        quint32 neighborSeqNo =  iter.value().toUInt();

        // case2 - I need to receive info
        if (!nested.contains(neighborOrigin) || nested[neighborOrigin].toUInt() < neighborSeqNo) {
        	//Send Status
					sendStatus(port);
          return;
        }
    }

    return;


}
void ChatDialog::sendRumor(QString myOrigin,QString mySeqNo, quint16 myPort){
	QVariantMap map;
	QByteArray datagram;
	QString Origin=myOrigin;
	QString seqNo=mySeqNo;
	quint16 port=myPort;
	QVariantMap tmp=qvariant_cast<QVariantMap>(oldMessagesCollection[Origin]);
	QString text=tmp[seqNo].toString();


	map["ChatText"]=QVariant(text);
	map["Origin"]=QVariant(Origin);
	map["SeqNo"]=QVariant(seqNo);

	//Creates Stream
	QDataStream outStream(&datagram, QIODevice::WriteOnly);
	outStream << map;

  socket->writeDatagram(datagram, QHostAddress("127.0.0.1"), port);

}
void ChatDialog::sendStatus(quint16 myPort){
	QByteArray datagram;
	QDataStream outStream(&datagram, QIODevice::WriteOnly);
	outStream << status;
	socket->writeDatagram(datagram, QHostAddress("127.0.0.1"), myPort);
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
			port = p;
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
