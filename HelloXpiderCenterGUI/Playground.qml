import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Rectangle {
    id:playground_
    property var xpider_queue_ : new Array()
    property var target_queue_ : new Array()
    property var cross_queue_ : new Array()

    property var max_xpider_num:100
    property var real_width:3.0 //3 meter
    property var real_height:2.0 //2 meter

    property var selected_xpider_id:-1
    property var reset_pos:-500


    function createXpider(num){
        var component = Qt.createComponent("Xpider.qml");
        if(component.status===Component.Ready){
            for(var i=0;i<num;++i){
                var dynamic_comp_ = component.createObject(playground_,
                                                       {"x":reset_pos,
                                                        "y":reset_pos,
                                                        "dev_id":i,
                                                        "playground":playground_,
                                                        "z":10,
                                                        "visible":false,});
                xpider_queue_.push(dynamic_comp_);
            }
        }
    }
    function createTarget(num){
        var component = Qt.createComponent("Target.qml");
        if(component.status===Component.Ready){
            for(var i=0;i<num;++i){
                var dynamic_comp_ = component.createObject(playground_,
                                                       {"x":reset_pos,
                                                        "y":reset_pos,
                                                        "dev_id":i,
                                                        "z":5,
                                                        "visible":true,});
                target_queue_.push(dynamic_comp_);
            }
        }
    }
    function createCross(num){
        var component = Qt.createComponent("Cross.qml");
        if(component.status===Component.Ready){
            for(var i=0;i<num;++i){
                var dynamic_comp_ = component.createObject(playground_,
                                                       {"x":reset_pos,
                                                        "y":reset_pos,
                                                        "z":1,
                                                        "visible":true,});
                cross_queue_.push(dynamic_comp_);
            }
        }
    }

    function childrenClicked(index){
        //console.log("children:",index," selected")
        for(var i=0;i<xpider_queue_.length;++i){
            xpider_queue_[i].setSelected(i==index && !xpider_queue_[i].selected_);
        }
        if(index!=selected_xpider_id){
            selected_xpider_id = index;
        }else{
            selected_xpider_id = -1;
        }
        console.log("selected xpider index:",selected_xpider_id)
    }

    MouseArea{
        anchors.fill: parent
        onClicked: {
            if(selected_xpider_id>=0){
                target_queue_[selected_xpider_id].x = mouse.x
                target_queue_[selected_xpider_id].y = mouse.y
                target_queue_[selected_xpider_id].show();

                //push target to c++
                var real_x = (mouse.x/playground_.width-0.5)*real_width;
                var real_y = (0.5-mouse.y/playground_.height)*real_height;
                opti_server_.pushTarget(selected_xpider_id,real_x,real_y);
                console.log("id:",selected_xpider_id," x:",real_x," y:",real_y);
            }
        }
    }
    Timer{
        id:timer
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            for(var i=0;i<target_queue_.length;++i){
                if(target_queue_[i].x<0 || xpider_queue_[i].x<0)continue

                var dx = (target_queue_[i].x-xpider_queue_[i].x)*(target_queue_[i].x-xpider_queue_[i].x);
                var dy = (target_queue_[i].y-xpider_queue_[i].y)*(target_queue_[i].y-xpider_queue_[i].y);
                var d = Math.sqrt(dx+dy);
                if(d<50){
                    opti_server_.removeTarget(i);
                    target_queue_[i].x = reset_pos;
                    target_queue_[i].y = reset_pos;
                }
            }
        }
    }
    function resetAllTargets(){
        for(var i=0;i<target_queue_.length;++i){
            opti_server_.removeTarget(i);
            target_queue_[i].x = reset_pos;
            target_queue_[i].y = reset_pos;
        }
    }

    Connections{
        target: opti_server_
        onXpiderUpdate:{
            //console.log("index:",id," theta:",theta," x:",x," y:",y)
            if(id>=0 && id<xpider_queue_.length){
                var screen_x = (x/real_width+0.5)*playground_.width
                var screen_y = (0.5-y/real_height)*playground_.height
                xpider_queue_[id].x = screen_x;
                xpider_queue_[id].y = screen_y;
                var angle = (90-theta*180.0/Math.PI)%360
                xpider_queue_[id].rotation=angle;
                if(is_real==false){
                    xpider_queue_[id].dev_id = -1;
                }else{
                    xpider_queue_[id].dev_id = id;
                }
                xpider_queue_[id].visible=true;
            }
        }
        onLandmarkUpdate :{
            console.log("corss udpated");
            if(id>=0 && id<cross_queue_.length){
                var screen_x = (x/real_width+0.5)*playground_.width
                var screen_y = (0.5-y/real_height)*playground_.height
                cross_queue_[id].x = screen_x;
                cross_queue_[id].y = screen_y;
            }
        }
    }
    Connections{
        target: xpider_center_
        onXpiderStarted:{
            createXpider(max_xpider_num)
            createTarget(max_xpider_num)
            createCross(max_xpider_num)
            console.log("xpdier queue intialized:",xpider_queue_.length);
            console.log("target queue intialized:",target_queue_.length);
            console.log("cross queue intialized:",cross_queue_.length);
        }

        onXpiderStoped:{
            console.log("xpider queue destroyed ")
        }
    }
}
