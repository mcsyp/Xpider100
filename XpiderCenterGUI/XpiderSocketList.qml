import QtQuick 2.0

import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.0

Window {
  id: xpider_status
  width: 400
  height: 900

  ListModel{
    id:xpider_socket_model
  }

  ListView {
    anchors.fill:parent
    model:xpider_socket_model
    delegate: Text {
      text: model.hostname + ":\t" + model.heartbeat
      font.pointSize: 12
    }
  }

  Connections {
    target:opti_server_
    onUpdateXpiderSocket: {
      xpider_socket_model.clear()
      for(var elm in map) {
        xpider_socket_model.append({"hostname": elm, "heartbeat":map[elm]})
      }
    }
  }
}
