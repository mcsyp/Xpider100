import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
Window {
    minimumHeight: 200
    minimumWidth: 400

    title: "Xpider Command Input:"
    Rectangle{
        id: info_rect_
        width:parent.width
        height:15
        color:"Black"
        visible: false
        Label{
            id: info_lbl_
            anchors.fill: parent
            font.pixelSize: 10
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color:"White"
        }
        SpringAnimation{
            id:animate_scale
            target:info_rect_
            property: "y"
            spring: 3.0
            damping: 0.2
            epsilon: 0.2
            from:-20
            to:0
            duration:30
        }
        Timer{
            id:v_timer_
            repeat: false
            interval: 3000
            onTriggered: {
                info_rect_.visible=false;
            }
        }

        function show(message,last){
            info_rect_.visible=true;
            info_lbl_.text = message;
            animate_scale.running=true
            v_timer_.interval = last;
            v_timer_.start();
        }
    }

    Flickable {
        id: flick
        clip: true
        y:info_rect_.height
        width: parent.width;
        height: parent.height-30;
        contentWidth: cmd_edit.paintedWidth
        contentHeight: cmd_edit.paintedHeight
        function ensureVisible(r)
        {
            if (contentX >= r.x)
                 contentX = r.x;
            else if (contentX+width <= r.x+r.width)
                 contentX = r.x+r.width-width;
            if (contentY >= r.y)
                 contentY = r.y;
            else if (contentY+height <= r.y+r.height)
                contentY = r.y+r.height-height;
        }

        TextEdit {
            id: cmd_edit
            width: flick.width
            height: flick.height
            focus: true
            wrapMode: TextEdit.Wrap
            onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
        }
     }

    Button{
        id:run_btn_
        text:"Run"
        height:25
        width:120
        x:parent.width-width
        y:(parent.height-height)
        onClicked: {
            if(cmd_edit.text.length==0){
                info_rect_.show("Empty commands!",3000)
                return
            }
            //push the command to center
            opti_server_.runCommandText(cmd_edit.text.trim());
            info_rect_.show("command accepted & running...",3000);
        }
    }
    Button{
        id:clear_btn_
        text:"Clear"
        height:25
        width:60
        x:run_btn_.x-clear_btn_.width-5
        y:(parent.height-height)
        onClicked: {
           cmd_edit.text="";
        }
    }
}
