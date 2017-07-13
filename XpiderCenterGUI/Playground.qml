import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Rectangle {
    id:playground_
    property var xpider_queue_ : new Array()
    property var target_queue_ : new Array()
    property var cross_queue_ : new Array()

    property var max_xpider_num_:100
    property var real_width_:3.5 //3 meter
    property var real_height_:3.5 //2 meter

    property var reset_pos_:-500


    function createXpider(num){
        var component = Qt.createComponent("Xpider.qml");
        if(component.status===Component.Ready){
            for(var i=0;i<num;++i){
                var dynamic_comp_ = component.createObject(playground_,
                                                       {"x":reset_pos_,
                                                        "y":reset_pos_,
                                                        "dev_id":i,
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
                                                       {"x":reset_pos_,
                                                        "y":reset_pos_,
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
                                                       {"x":reset_pos_,
                                                        "y":reset_pos_,
                                                        "z":2,
                                                        "visible":true,});
                cross_queue_.push(dynamic_comp_);
            }
        }
    }
    function convertFromRealToScreen(real_x,real_y){
        var screen_x,screen_y;
        screen_x = (real_x/real_width_+0.5)*main_window_.width
        screen_y = (0.5-real_y/real_height_)*main_window_.height
        return [screen_x,screen_y];
    }
    function convertFromScreenToReal(screen_x,screen_y){
        var real_x,real_y;
        real_x = (screen_x/main_window_.width-0.5)*real_width_;
        real_y = (0.5-screen_y/main_window_.height)*real_height_;
        return [real_x,real_y];
    }

    Timer{
        id:target_timer
        repeat: true
        interval: 1000
        onTriggered: {

        }
    }

    MouseArea{
        anchors.fill: parent        
        property var selected_xpider_index_:-1
        onClicked: {
            var index = selectXpider(mouse.x,mouse.y);
            //console.log("selected index:",selected_xpider_index_," mouse_index:",index);
            if(selected_xpider_index_>=0 && index<0 && index!==-2){
                var dev_id = xpider_queue_[selected_xpider_index_].dev_id;

                //push target to c++
                var real_pos = convertFromScreenToReal(mouse.x,mouse.y);
                opti_server_.pushTarget(dev_id,real_pos[0],real_pos[1]);
                //console.log("selected target id:",dev_id," x:",real_pos[0]," y:",real_pos[1]);
            }else{
                selected_xpider_index_ = index;
            }
        }
    }
    function selectXpider(x,y){
        var min_dis=1000;
        var min_index=0;
        for(var i=0;i<xpider_queue_.length;++i){
            var xpider = xpider_queue_[i];
            var d = Math.abs(x-xpider.x)+Math.abs(y-xpider.y);
            if(d<min_dis){
                min_dis = d;
                min_index=i;
            }
        }

        //console.log("min_dis is:",min_dis);
        if(min_dis<40 && xpider_queue_[min_index].dev_id>=0){
            for(var i=0;i<xpider_queue_.length;++i){
                var selected = (i===min_index && !xpider_queue_[i].selected_);
                xpider_queue_[i].setSelected(selected);
            }
            if(xpider_queue_[min_index].selected_) {
                return min_index;
            }else{//cancel selection
                opti_server_.removeTarget(xpider_queue_[min_index].dev_id);
                return -2;
            }
        }
        return -1;
    }

    function resetAllTargets(){
        opti_server_.clearTargets()
    }

    Connections{
        target: opti_server_
        onXpiderListUpdate:{
            var xpider_list = JSON.parse(str_json);
            for(var i=0;i<xpider_queue_.length;++i){
                if(i<xpider_list.length){
                    var xpider = xpider_list[i];

                    //show xpider
                    var screen_pos=convertFromRealToScreen(xpider.x,xpider.y);
                    xpider_queue_[i].x = screen_pos[0];
                    xpider_queue_[i].y = screen_pos[1];
                    var angle = (90-xpider.theta*180.0/Math.PI)%360
                    xpider_queue_[i].rotation=angle;
                    xpider_queue_[i].dev_id = xpider.id;
                    xpider_queue_[i].label_ = xpider.label;
                    xpider_queue_[i].visible = true;

                    //show target
                    if(xpider.id>=0){
                        screen_pos=convertFromRealToScreen(xpider.target_x,xpider.target_y);
                        target_queue_[i].x = screen_pos[0]
                        target_queue_[i].y = screen_pos[1]
                        target_queue_[i].dev_id = xpider.id;
                        target_queue_[i].visible=true;
                        //console.log("id:",xpider.id,
                        //            " x:",xpider_queue_[i].x,
                        //            " y:",xpider_queue_[i].y,
                        //            " target_x:",target_queue_[i].x,
                        //            " target_y:",target_queue_[i].y);
                    }
                }else{
                    xpider_queue_[i].visible = false;
                    target_queue_[i].visible = false;
                }
            }
        }
        onLandmarkListUpdate:{
            var mark_list = JSON.parse(str_json);
            console.log("landmarks:",mark_list.length);
            for(var i=0;i<cross_queue_.length;++i){
                if(i<mark_list.length){
                    var mark= mark_list[i];
                    var screen_pos = convertFromRealToScreen(mark.x,mark.y);
                    cross_queue_[i].x = screen_pos[0];
                    cross_queue_[i].y = screen_pos[1];
                    cross_queue_[i].cross_id = mark.id;
                    cross_queue_[i].visible = true;
                }else{
                    cross_queue_[i].visible = false;
                }
            }
        }

        onServiceInitializing:{
            createXpider(max_xpider_num_)
            createTarget(max_xpider_num_)
            createCross(max_xpider_num_)
            console.log("xpdier queue intialized:",xpider_queue_.length);
            console.log("target queue intialized:",target_queue_.length);
            console.log("cross queue intialized:",cross_queue_.length);
        }
    }
}
