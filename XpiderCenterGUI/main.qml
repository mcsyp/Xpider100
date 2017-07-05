import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id:main_window_
    visible: true
    width:1200
    height:800
    minimumWidth: 900
    minimumHeight: 600

    Playground {
     id:playground_
     anchors.fill: parent
     anchors.centerIn: parent
    }
    onWidthChanged: {
        main_window_.height = main_window_.width*2/3;
    }

    onClosing: {
        opti_server_.StopService()
    }


    Button{
        id:start_btn_
        text:"Stop"
        x:parent.width/2-start_btn_.width/2;
        y:parent.height-start_btn_.height-5
        onClicked: {
            playground_.resetAllTargets()
        }
    }

}
