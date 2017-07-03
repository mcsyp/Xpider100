import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    visible: true
    width:1200
    height:800
    minimumWidth: 900
    minimumHeight: 600

    Playground {
     id:playground_
     width:parent.width
     height:parent.width*2/3;
     anchors.centerIn: parent
    }

    onClosing: {
        xpider_center_.StopConnection()
    }

    Button{
        id:start_btn_
        text:"Start"
        property bool is_active: false
        x:parent.width/2-start_btn_.width/2;
        y:parent.height-start_btn_.height-5
        onClicked: {
            is_active = !is_active
            if(is_active===false){
                start_btn_.text = "Start"
            }else{
                start_btn_.text = "Stop"
            }
        }
    }
}
