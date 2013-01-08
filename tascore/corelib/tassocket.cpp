/*************************************************************************** 
** 
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
** All rights reserved. 
** Contact: Nokia Corporation (testabilitydriver@nokia.com) 
** 
** This file is part of Testability Driver Qt Agent
** 
** If you have questions regarding the use of this file, please contact 
** Nokia at testabilitydriver@nokia.com . 
** 
** This library is free software; you can redistribute it and/or 
** modify it under the terms of the GNU Lesser General Public 
** License version 2.1 as published by the Free Software Foundation 
** and appearing in the file LICENSE.LGPL included in the packaging 
** of this file. 
** 
****************************************************************************/ 



#include <QDebug>    
#include <QStack>    
#include <QObject>
#include <QWeakPointer>

#include "tascoreutils.h"
#include "tassocket.h"
#include "taslogger.h"

/*!
    \class TasSocket
    \brief TasSocket represents the socket communication between the server and client.
    
    Plugins and and test framework communication with the server using tcp socket communication.
    The TasSocket class takes care of the communication details including messaging and connection
    details.


    Messages are parsed and generated acording to the small protocol defined between tas modules.
    The protocol is very simple it consists of a header and body.
    Message Header
    Flag: 1 byte
    Body size: 4 bytes
    Crc of the body: 2 bytes
    Compressed: 1 byte
    Body: Body size bytes.
    
    The messages will be formatted in little endian byte order.
    
    All communication is done synchronously so that the messaging will be
    expected to fail incase the given timeout value is exceeded. The default
    value is one second so operation expected to take longer must use
    larger values.
    
 */

static const int CHUNK_SIZE = 1024;
static const int SLEEP_TIME = 5;

TasServerSocket::TasServerSocket(QIODevice* device, QObject *parent)
    :TasSocket(device, parent)
{
    clearHandlers();
}


void TasServerSocket::closeConnection()
{
    closeDevice();
}

void TasServerSocket::deviceDisconnected()
{
    emit socketClosed();
    deleteLater();
}

void TasServerSocket::setIdentification(const QString& identification)
{
    mIdentification = identification;
}


TasClientSocket::TasClientSocket(QIODevice* device, QObject *parent)
    :TasSocket(device, parent)
{
}


void TasClientSocket::closeConnection()
{
    closeDevice();
}

void TasClientSocket::deviceDisconnected()
{
    emit socketClosed();
}

/*!
    Construct a new TasSocket.
*/
TasSocket::TasSocket(QIODevice* device, QObject *parent)
    :QObject(parent)
{  
    mDevice = device;
    clearHandlers();

    mReader = new TasSocketReader(device, this);
    mWriter = new TasSocketWriter(device, this);

    connect(mDevice, SIGNAL(aboutToClose()), this, SLOT(deviceDisconnected()));
    connect(mReader, SIGNAL(messageRead(TasMessage&)), this, SLOT(messageAvailable(TasMessage&)));
    connect(mDevice, SIGNAL(destroyed(QObject*)), this, SLOT(cleanUp(QObject*)));
}                           

/*!
    Destructor
*/
TasSocket::~TasSocket()
{    
    cleanUp();
    delete mReader;
    delete mWriter;
}

void TasSocket::closeDevice()
{
    if (mDevice->isOpen()) {
        mDevice->close();
    }
}

void TasSocket::cleanUp(QObject*)
{
    mReader->close();
    mWriter->close();
    mDevice = 0;
}

/*!
  Set the request handler for the socket. The incoming message is passed to
  the handler and the resulting response is sent back to the socket
  as the response to the request.
  The socket does not assume ownership of the handler so make sure you do not
  remove the handler when the socket is still alive.
  Use clearHandlers to remove handlers from the socket. You can also
  reset this handler only passing 0 as argument(e.g. setRequestHandler(0))
 */
void TasSocket::setRequestHandler(RequestHandler* handler)
{
    mRequestHandler = handler;
}

/*!
  Set the response handler for the socket. The incoming message is passed to
  the handler.
  The socket does not assume ownership of the handler so make sure you do not
  remove the handler when the socket is still alive.
  Use clearHandlers to remove handlers from the socket. You can also
  reset this handler only passing 0 as argument(e.g. setResponseHandler(0))
 */
void TasSocket::setResponseHandler(ResponseHandler* handler)
{
    mResponseHandler = handler;
}

void TasSocket::clearHandlers()
{
    mResponseHandler = 0;
    mRequestHandler = 0;
}

/*!
 * Request data synchronously.
 */
bool TasSocket::syncRequest(const qint32& messageId, const QString& requestMessage, TasMessage &reply)
{
    return syncRequest(messageId, QByteArray(requestMessage.toUtf8()), reply);
}

/*!
 * Request data synchronously.
 */
bool TasSocket::syncRequest(const qint32& messageId, const QByteArray& requestMessage, TasMessage &reply)
{
    //disconnect response reader
    disconnect(mDevice, SIGNAL(readyRead()), mReader, SLOT(readMessageData()));

    //send request as normal
    bool success = sendRequest(messageId, requestMessage);

    //read response directly
    while (success) {
        if(!mDevice->waitForReadyRead ( READ_TIME_OUT )){
            TasLogger::logger()->error("TasSocket::syncRequest timeout when waiting for the response");
            //return empty data
            success = false;
        }
        else if(mDevice->bytesAvailable() >= HEADER_LENGTH){
            // got header, exit loop with success==true
            break;
        }
    }

    //read message if all ok
    if(success) {
        if(mReader->readOneMessage(reply)){
            if(reply.messageId() != messageId) {
                //not the message we wanted
                messageAvailable(reply);
                TasLogger::logger()->error("TasSocket::syncRequest message was not responded.");
                success = false;
            }
            else {
                reply.uncompressData();
                Q_ASSERT(success);
            }
        }
        else{
            TasLogger::logger()->error("TasSocket::syncRequest error when reading message.");
            success = false;
        }
    }
    //reconnect response reader
    connect(mDevice, SIGNAL(readyRead()), mReader, SLOT(readMessageData()));
    return success;
}


/*!
  Send a message over the connection represented by this socket. Returns false if the connection
  is not writable.
*/
bool TasSocket::sendRequest(const qint32& messageId, const QString& message)
{
    return sendRequest(messageId, QByteArray(message.toUtf8()));
}


/*!
  Send a message over the connection represented by this socket. Returns false if the connection
  is not writable. Message will be deleted once message is sent.
*/
bool TasSocket::sendRequest(const qint32& messageId, const QByteArray& message)
{
    TasMessage msg(REQUEST_MSG, false, message, messageId);
    return sendMessage(msg);
}

/*!
    Send a response type mesage over the socket. Response are to be send only
    as response to requests. This function will not wait for a response
    and will return true once the message bytes are written. False returned
    only if there is no connection.
    If the compressed flag is set to true then it is assumed that the message data
    is compressed already. This means that the protocol will indicate that the
    message is compressed but this function will not perform the compression.
*/
bool TasSocket::sendResponse(const qint32& messageId, const QByteArray& message, bool compressed)
{
    TasMessage msg(RESPONSE_MSG, compressed, message, messageId);
    return sendMessage(msg);
}

bool TasSocket::sendResponse(const qint32& messageId, const QString& message, bool compressed)
{
    return sendResponse(messageId, QByteArray(message.toUtf8()), compressed);
}


bool TasSocket::sendError(const qint32& messageId, const QString& message)
{
    return sendError(messageId, QByteArray(message.toUtf8()));
}

bool TasSocket::sendError(const qint32& messageId, const QByteArray& message, bool compressed)
{
    //wil delete the message
    TasMessage msg(ERROR_MSG, compressed, message, messageId);
    return sendMessage(msg);
}

bool TasSocket::sendMessage(TasMessage& message)
{
    bool ok = mWriter->writeMessage(message);
    emit messageSent();
    return ok;
}

/*!
    When the socket is not in the process of sending or reading messages
    this slot listens to incoming messages by listening to the readyRead
    signal.
*/
//void TasSocket::dataAvailable()
void TasSocket::messageAvailable(TasMessage& message)
{ 
    if (message.isRequest() && mRequestHandler) {
        qDebug() << mRequestHandler << message.dataAsString();
        mRequestHandler->serviceRequest(message, this);
    } else if (message.isResponse() && mResponseHandler) {
        qDebug() << mResponseHandler << message.dataAsString();
        mResponseHandler->serviceResponse(message);
    } else {
        TasLogger::logger()->warning("TasSocket::dataAvailable Received a message: "
                                     + QString::number(message.flag()) + " but no handlers.");
    }
}

TasSocketWriter::TasSocketWriter(QIODevice* device, QObject* parent)
    :QObject(parent)
{
    mDevice = device;
    mTcpSocket = 0;
    mLocalSocket = 0;

    QAbstractSocket* tcpSocket = qobject_cast<QAbstractSocket*>(mDevice);
    if (tcpSocket) {
        mTcpSocket = tcpSocket;
    } else {
        QLocalSocket* socket = qobject_cast<QLocalSocket*>(mDevice);
        if (socket) {
            mLocalSocket = socket;
        }
    }
}

TasSocketWriter::~TasSocketWriter()
{
    close();
}

bool TasSocketWriter::writeMessage(TasMessage& message)
{
    if(!mDevice || !mDevice->isWritable()){
        TasLogger::logger()->error("TasSocket::writeMessage socket not writable, cannot send message!" + QString::number(message.messageId()));
        return false;
    }
    QByteArray header = makeHeader(message);
    mDevice->write(header.data(), header.size());
    writeBytes(message.dataCompressed());
    return true;
}

void TasSocketWriter::writeBytes(const QByteArray& msgBytes)   
{
#ifdef Q_OS_SYMBIAN
    //write the data in one kb chunks to avoid problems caused by unstable hw
    int bytesLeft = msgBytes.size();
    int chunksWritten = 0;
    forever{
        if(bytesLeft > 0){
            bytesLeft -= mDevice->write(msgBytes.mid(chunksWritten*CHUNK_SIZE).data(), qMin(CHUNK_SIZE,bytesLeft));
            flush();
            TasCoreUtils::wait(SLEEP_TIME);
        }
        else{
            break;
        }
        chunksWritten++;
    }
#else
    mDevice->write(msgBytes.data(), msgBytes.size());
#endif
    mDevice->waitForBytesWritten(READ_TIME_OUT);
}

void TasSocketWriter::close()
{
    mDevice = 0;
}

/*!
  In some symbian devices comms freeze if not flushed.
 */
void TasSocketWriter::flush()
{
    if(mLocalSocket){
        mLocalSocket->flush();
    }
    else if(mTcpSocket){
        mTcpSocket->flush();
    }
}

/*!
    Construt a  message from the given flag and message bytes.
 */
QByteArray TasSocketWriter::makeHeader(TasMessage& message)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::LittleEndian);
    out.setVersion(QDataStream::Qt_5_0);
    QByteArray payload = message.dataCompressed();
    quint16 bodyCrc =  qChecksum(payload.data(), payload.size());
    out << message.flag() << payload.size() << bodyCrc << COMPRESSION_ON << message.messageId();
    return block;
}

TasSocketReader::TasSocketReader(QIODevice* device, QObject* parent)
    :QObject(parent)
{
    mDevice = device;
    connect(mDevice, SIGNAL(readyRead()), this, SLOT(readMessageData()));
}

TasSocketReader::~TasSocketReader()
{
    if (mDevice) {
        close();
    }
}

void TasSocketReader::close()
{       
    mDevice->disconnect(this);
    mDevice = 0;
}

/*!
    Reads a message from the socket and places it in the response QByteArray. Will return false
    if reading the message fails.
    The returnRaw flag means that the message body will not be touched if it is compressed.
    The returnRaw flag is to be used with caution since passing the message on without knowledge of the
    compression may cause problems.
*/
void TasSocketReader::readMessageData()
{   
    TasLogger::logger()->debug("TasSocketReader::readMessageData start.");
    if (!mDevice) {
        TasLogger::logger()->error("TasSocketReader::readMessageData reading device not available.");
    }
    //wait for header data to be available, start process only after
    //enough data available
    if (mDevice->bytesAvailable() < HEADER_LENGTH) {
        return;
    }

    disconnect(mDevice, SIGNAL(readyRead()), this, SLOT(readMessageData()));

    TasMessage message;
    if (readOneMessage(message)) {
        emit messageRead(message);
    }

    if (mDevice) {
        connect(mDevice, SIGNAL(readyRead()), this, SLOT(readMessageData()));
        //maybe there was a new message coming when the old one was still being processed.
        if (mDevice->bytesAvailable() > 0) {
            readMessageData();
        }
    }
}

/*!
 * Read on message from the iodevice
 */
bool TasSocketReader::readOneMessage(TasMessage& message)
{
    quint8 compressed = 0;
    qint32 bodySize = 0;
    quint16 crc = 0;
    quint8 flag = 0;
    qint32 messageId = 0;
    QDataStream in(mDevice);
    in.setVersion(QDataStream::Qt_5_0);
    in.setByteOrder(QDataStream::LittleEndian);

    //read header
    in >> flag >> bodySize >> crc >> compressed >> messageId;
    
    bool compression = false;
    if (compressed == COMPRESSION_ON) {
        compression = true;
    }

    bool ok = true;

    //read body, use weakpointer to try to detect sudden disconnections
    QByteArray rawBytes;
    
    int totalBytes = 0;
    forever{
        if(mDevice->bytesAvailable() == 0){
            if(!mDevice->waitForReadyRead(READ_TIME_OUT)){
                TasLogger::logger()->error("TasSocket::readData error when waiting for more data. " + mDevice->errorString());
                TasLogger::logger()->error("TasSocket::readData bytes read: "+QString::number(totalBytes)
                                           + " bytes exptected: "+ QString::number(bodySize));
                ok = false;
                break;
            }
        }
        
        if(mDevice->bytesAvailable() < 0){
            TasLogger::logger()->error("TasSocket::readData error in reading data. ");
            ok = false;
            break;
        }

        int available = mDevice->bytesAvailable();
        if( (totalBytes + available) > bodySize ){
            available = bodySize - totalBytes;
        }
        char* buffer = (char*)malloc(available);
        int bytesRead = in.readRawData(buffer, available);
        rawBytes.append(buffer, bytesRead);
        free(buffer);
        totalBytes += bytesRead;
        if(totalBytes >= bodySize){
            break;
        }

    }

    bool messageRead = false;

    if(ok && !rawBytes.isEmpty()){
        //check crc
        quint16 checkSum = qChecksum( rawBytes.data(), bodySize );
        if ( checkSum != crc){
            TasLogger::logger()->error("TasSocket::readMessageData CRC error " + QString::number(checkSum) + " read: "
                                       + QString::number(crc));
        }
        else{
            message.setMessageId(messageId);
            message.setFlag(flag);
            message.setData(rawBytes, compression);
            messageRead = true;
        }
    }
    return messageRead;
}

