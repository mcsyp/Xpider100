/**
 * Author: Ye Tian <flymaxty@foxmail.com>
 * Copyright (c) 2016 Maker Collider Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 */

#ifndef XPIDER_COMM_H_
#define XPIDER_COMM_H_

#include <QTimer>
#include <QObject>
#include <QThread>
#include <QTcpSocket>

#include "hdlc_qt.h"

class XpiderComm: public QObject {
  Q_OBJECT

public:
    static const QByteArray kDataHeader;

public:
    XpiderComm(QObject *parent = 0);
    ~XpiderComm();

    void SendFrame(QByteArray data);
    void Connect(const QString &host_name, const quint16 &host_port);
    void Disconnect();

signals:
    void netConnected();                                                 // Emit when socket connected
    void newFrame(QByteArray);                                        // Emit when get new frame
    void error(const int &socketError, const QString &message);       // Emit when socket has error

    void connectHost();                                               // Emit when call public function Connect()
    void disconnectHost();

private slots:
    void HeartBeat();                                                 // Send heartbeat when timer triggered.
    void ConnectSlot();                                               // Will be called when connectHost emit
    void DisconnectSlot();                                            // Will be called when disconnectHost emit
    void ReadyRead();                                                 // Will be called when socket has data to read
    void Send(QByteArray data);                                       // Will be called by hdlc hdlcTransmitByte signal
    void GetFrame(QByteArray data, quint16 data_length);              // Will be called when hdlc get a vaild frame
    void ErrorHandle(const int &socketError, const QString &message); // Will be called when socket has error

private:
    bool connected_;

    bool heartbeat_;
    QTimer *heartbeat_timer_;

    QString host_name_;
    quint16 host_port_;

    HDLC_qt* hdlc_;
    QTcpSocket* socket_;
};

#endif // XPIDER_COMM_H_
