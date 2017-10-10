import QtQuick 2.0

import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.0

Window {
  id: xpider_status
  width: 300
  height: 800

  ListModel{
    id:xpider_socket_model
  }

  Text{
    id:list_txt_components

  }

  ListView {
    anchors.fill:parent
    model:xpider_socket_model
    delegate: Text {
      text: model.hostname + ":\t" + model.heartbeat
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
