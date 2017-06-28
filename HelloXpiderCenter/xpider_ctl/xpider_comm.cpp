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

#include "xpider_comm.h"

const QByteArray XpiderComm::kDataHeader = QByteArray::fromHex("0155");

XpiderComm::XpiderComm(QObject *parent): QObject(parent) {
  connected_ = false;

  hdlc_ =HDLC_qt::instance();
  connect(hdlc_, &HDLC_qt::hdlcValidFrameReceived, this, &XpiderComm::GetFrame);
  connect(hdlc_, SIGNAL(hdlcTransmitByte(QByteArray)), this, SLOT(Send(QByteArray)));

  connect(this, SIGNAL(connectHost()), this, SLOT(ConnectSlot()));
  connect(this, SIGNAL(disconnectHost()), this, SLOT(DisconnectSlot()));
  connect(this, &XpiderComm::error, this, &XpiderComm::ErrorHandle);
}

XpiderComm::~XpiderComm() {
  DisconnectSlot();
}

void XpiderComm::Connect(const QString &host_name, const quint16 &host_port) {
  host_name_ = host_name;
  host_port_ = host_port;

  emit connectHost();
}

void XpiderComm::Disconnect() {
  emit disconnectHost();
}

void XpiderComm::ConnectSlot() {
  socket_ = new QTcpSocket;
  socket_->connectToHost(host_name_, host_port_);

  if (!socket_->waitForConnected(3000)) {
    connected_ = false;

    emit error(socket_->error(), socket_->errorString());
  } else {
    connected_ = true;
    connect(socket_, &QIODevice::readyRead, this, &XpiderComm::ReadyRead);
    heartbeat_timer_ = new QTimer(this);
    connect(heartbeat_timer_, &QTimer::timeout, this, &XpiderComm::HeartBeat);
    heartbeat_timer_->start(5000);
    emit netConnected();

    // qDebug() << "(XpiderComm) Connected!";
  }
}

void XpiderComm::DisconnectSlot() {
  if(connected_) {
    if(heartbeat_timer_ != NULL) {
      heartbeat_timer_->stop();
      delete heartbeat_timer_;
    }
    socket_->disconnect();
    if(!socket_->waitForDisconnected(3000)) {
      emit error(socket_->error(), socket_->errorString());
    } else {
      // qDebug() << "(XpiderComm) Disconnect Success!";
    }
  }
}

void XpiderComm::ReadyRead() {
  QByteArray raw_data;
  // raw_data = socket_->read(50);
  raw_data = socket_->readAll();
  if(raw_data.data()[0]==0x01 && raw_data.data()[1]==0x55) {
    // Remove special data header for rakvideo
    raw_data.remove(0, 2);

    hdlc_->charReceiver(raw_data);
  }

  // qDebug() << "(XpiderComm) Receive raw data: " << raw_data.toHex();
  // qDebug() << "(XpiderComm) Receive raw string: " << QString(raw_data);
}

void XpiderComm::GetFrame(QByteArray data, quint16 data_length) {
  // Cut off useless data
  data.remove(data_length, data.length()-data_length);

  emit newFrame(data);

  // qDebug() << "(XpiderComm) Receive frame: " << data.toHex() << ", length" << data.length();
}

void XpiderComm::ErrorHandle(const int &socketError, const QString &message) {
  Q_UNUSED(socketError);
  Q_UNUSED(message);
  // qDebug() << "(XpiderComm) Error: " << socketError << ", " << message;
}

void XpiderComm::HeartBeat() {
  if(!heartbeat_) {
    /* Use more effective way to keep connections */
    Send(QByteArray::fromStdString("   "));
  } else {
    heartbeat_ = false;
  }
}

void XpiderComm::SendFrame(QByteArray data) {
  hdlc_->frameDecode(data, data.length());

  // qDebug() << "(XpiderComm) Send raw: " << data.toHex();
}

void XpiderComm::Send(QByteArray data) {
  int send_result;

  // Add special data header for rakvideo
  data.insert(0, kDataHeader);
  send_result = socket_->write(data);
  if(send_result == -1) {
    emit error(socket_->error(), socket_->errorString());
  }
  heartbeat_ = true;

  // qDebug() << "(XpiderComm) Send data: " << data.toHex() << ", result: " << send_result;
}
