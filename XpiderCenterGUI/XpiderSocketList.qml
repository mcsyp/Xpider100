import QtQuick 2.0

import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
Window {
    ListModel{
        id:xpider_socket_model
    }
    Text{
        id:list_txt_component
        text: hostname+":"+hostport+"/"+heartbeat
    }

    ListView {
        anchors.fill:parent
        model:xpider_socket_model
        delegate: list_txt_components
    }
    Connections{
        target:opti_server_
        onUpdateXpiderSocket:{
            var sock_list = JSON.parse(str_json);
            for(var i=0;i<sock_list.length;++i){
                var sock = sock_list[i];
                //console.log(sock)
            }
        }
    }
}
