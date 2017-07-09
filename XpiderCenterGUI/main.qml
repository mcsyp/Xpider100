import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id:main_window_
    visible: true
    width:1440
    height:900
    minimumWidth: width
    minimumHeight: height
    maximumWidth: width
    maximumHeight: height

    property var command_list_:new Array()

    Playground {
     id:playground_
     width:  parent.width
     height: width*2/3;
    }

    Button{
        id:start_btn_
        property bool is_planner_running:false
        text:(is_planner_running)?"Stop":"Start"
        x:main_window_.width/2-start_btn_.width-2;
        y:main_window_.height-start_btn_.height-5
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
        x:2+main_window_.width/2
        y:main_window_.height-cmd_btn_.height-5
        onClicked: {
            var panel = availableCommandPanel()
            panel.visible=true;
            panel.raise();
            console.log("command panel list len:",command_list_.length);
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
    /*
    CommandPanel{
        id:cmd_panel_;
        visible: false;
    }
    */

}
