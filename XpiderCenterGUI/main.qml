import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id:main_window_
    visible: true
    width:1560
    height:900
    minimumWidth: width
    minimumHeight: height
    maximumWidth: width
    maximumHeight: height

    property var command_list_:new Array()

    Playground {
        id:playground_
        anchors.fill: parent
    }

    Rectangle{
        id: opti_connection_state_rect_
        opacity: 0.7
        width:opti_connection_state_txt_.contentWidth+10
        height:opti_connection_state_txt_.contentHeight
        x:0
        y:main_window_.height/5
        property bool is_connected: false
        color: (is_connected)?"Orange":"White"

        Text{
            id: opti_connection_state_txt_
            anchors.fill: parent
            font.pixelSize: 15
            verticalAlignment: Text.AlignVCenter
            color:(opti_connection_state_rect_.is_connected)?"White":"Black"
            font.bold: true
            text: (opti_connection_state_rect_.is_connected)?"Optitrack online":"Optitrack offline";
        }
        SpringAnimation{
            id:opti_animate_scale
            target:opti_connection_state_rect_
            property: "x"
            spring: 3.0
            damping: 0.2
            epsilon: 0.2
            from:-opti_connection_state_rect_.width-50
            to:0
            duration:45
        }
        Connections{
            target:opti_server_
            onOptitrackConnected:{
                opti_connection_state_rect_.is_connected = connected;
                opti_animate_scale.running=true
            }
        }
    }


    Rectangle{
        id: xpider_number_rect_
        color: "Black"
        opacity: 0.60
        width:xpider_number_txt_.contentWidth+10
        height: xpider_number_txt_.contentHeight
        x:0
        y:opti_connection_state_rect_.y+opti_connection_state_rect_.height+3
        property int xpider_number_: 0
        visible:xpider_number_>0
        Text{
            id:xpider_number_txt_
            verticalAlignment: Text.AlignVCenter
            text: "< "+xpider_number_rect_.xpider_number_+" > online"
            font.pixelSize: 15
            color:"White"
        }
        SpringAnimation{
            id:xpider_animate_scale
            target:opti_connection_state_rect_
            property: "scale"
            spring: 3.0
            damping: 0.2
            epsilon: 0.2
            from:0.6
            to:1.0
            duration:30
        }
        Connections{
            target:opti_server_
            onXpiderAliveUpdate:{
                xpider_number_rect_.xpider_number_=number;
                xpider_animate_scale.running = true;
            }
        }
    }
    Rectangle{
        id: opti_number_rect_
        color: "Red"
        opacity: 0.80
        width:opti_number_txt_.contentWidth+10
        height: opti_number_txt_.contentHeight
        x:0
        y:xpider_number_rect_.y+xpider_number_rect_.height+3
        visible:(playground_.xpider_available_counter_+playground_.opti_counter_>0)
        Text{
            id:opti_number_txt_
            verticalAlignment: Text.AlignVCenter
            text: "< "+playground_.xpider_available_counter_+"/"+playground_.opti_counter_+" > detected"
            font.pixelSize: 12
            color:"White"
        }
    }
    Button{
        id:start_btn_
        property bool is_planner_running:false
        text:(is_planner_running)?"Stop":"Start"
        width:150
        x:main_window_.width-start_btn_.width
        y:main_window_.height/3
        z:5
        font.bold: true
        opacity: 0.8
        onClicked: {
            if(is_planner_running){
                playground_.resetAllTargets();
            }
            is_planner_running = !is_planner_running;
            opti_server_.enablePlanner(is_planner_running);
        }
    }
    Button{
        id:cmd_btn_
        text:"Command"
        width:150
        x:main_window_.width-cmd_btn_.width
        y:start_btn_.y+start_btn_.height+5
        z:5
        opacity: 0.8
        onClicked: {
            var panel = availableCommandPanel()
            panel.visible=true;
            panel.raise();
            console.log("command panel list len:",command_list_.length);
        }
    }

    Button {
        id:reset_landmarks_btn_
        width:150
        text: "Reset Landmarks"
        x:main_window_.width-reset_landmarks_btn_.width
        y:cmd_btn_.y+cmd_btn_.height+5
        z:5
        opacity: 0.8
        onClicked: {
            opti_server_.resetLandmarks();
            console.log("reset all landmarks")
        }
    }

    Button{
        id:save_btn_
        text:"Save Targets"
        width:150
        x:main_window_.width-save_btn_.width
        y:reset_landmarks_btn_.y+reset_landmarks_btn_.height+5
        z:5
        opacity: 0.8
        onClicked: {
            target_record_dialog_.is_saving_=true;
            target_record_dialog_.title="Saving targets to .CSV"
            target_record_dialog_.selectExisting=false
            target_record_dialog_.selectFolder=false
            target_record_dialog_.open()
        }
    }

    Button{
        id:load_btn_
        text:"Load Targets"
        width:150
        x:main_window_.width-load_btn_.width
        y:save_btn_.y+save_btn_.height+5
        z:5
        opacity: 0.8
        onClicked: {
            target_record_dialog_.is_saving_=false;
            target_record_dialog_.title="Loading .CSV target records."
            target_record_dialog_.selectExisting=true
            target_record_dialog_.selectFolder=false
            target_record_dialog_.open()
        }
    }

    FileDialog{
        id:target_record_dialog_
        title: "Please choose a path to save"
        folder: shortcuts.documents
        nameFilters: ["*.csv"]
        selectExisting: true
        selectFolder:false
        selectMultiple:false

        property  bool is_saving_: false
        onAccepted: {
            if(is_saving_){
                opti_server_.csvSaveTargets(target_record_dialog_.fileUrl)
            }else{
                opti_server_.csvLoadTargets(target_record_dialog_.fileUrl)
            }
        }

    }

    function createCommanPanel(){
        var component = Qt.createComponent("CommandPanel.qml");
        if(component.status===Component.Ready){
            for(var i=0;i<10;++i){
                var dynamic_comp_ = component.createObject(main_window_,
                                                       {"x":0,
                                                        "y":0,
                                                        "visible":false,});
                dynamic_comp_.x = i*100;
                dynamic_comp_.y = i*100;
                command_list_.push(dynamic_comp_);
            }
        }
    }

    function availableCommandPanel(){
        for(var i=0;i<command_list_.length;++i){
            var panel = command_list_[i];
            if(panel.visible===false){
                return panel;
            }
        }
        return command_list_[i];
    }
    Connections{
        target:opti_server_
        onServiceInitializing:{
             createCommanPanel();
             console.log("command panel list intialized:",command_list_.length);
         }
    }

    Component.onCompleted: {
      var component = Qt.createComponent("XpiderSocketList.qml");
      if(component.status===Component.Ready) {
        var dynamic_comp_ = component.createObject(main_window_,
                                               {"x":0,
                                                "y":0,
                                                "visible":true});
      }
    }
}
