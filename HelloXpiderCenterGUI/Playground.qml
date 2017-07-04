import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Rectangle {
    id:playground_
    property var xpider_queue_ : new Array()
    property Component my_component_:null
    property var max_xpider_num:100
    property var real_width:3.0 //3 meter
    property var real_height:2.0 //2 meter

    function createXpider(id){
        var posx,posy;
        var dynamic_comp_;
        if(my_component_==null){
            my_component_ = Qt.createComponent("Xpider.qml");
        }
        if(my_component_.status===Component.Ready){
            /*
            var seed= xpider_queue_.length;
            posx = 100+Math.random(seed)*(parent.width-200);
            posy = 100+Math.random(seed)*(parent.height-200);
            */
            posx = -500;
            posy = -200;
            dynamic_comp_ = my_component_.createObject(playground_,
                                                       {"x":posx,
                                                        "y":posy,
                                                        "dev_id":id,
                                                        "playground":playground_,
                                                        "z":2,
                                                        "visible":true,});
            //console.log("xpider:",id,"x:",posx,"y:",posy,"length:",xpider_queue_.length);
            xpider_queue_.push(dynamic_comp_);
            return dynamic_comp_;
        }
        return null;
    }

    function clearXpiderQueue(){
        if(xpider_queue_){
            for(var i=0;i<xpider_queue_.length;++i){
                xpider_queue_[i].destroy();
            }
            xpider_queue_=[];
        }
    }
    function childrenClicked(index){
        console.log("children:",index," selected")
        for(var i=0;i<xpider_queue_.length;++i){
            xpider_queue_[i].setSelected(i==index);
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
                xpider_queue_[id].dev_id = id;
                var angle = (90-theta*180.0/Math.PI)%360
                xpider_queue_[id].rotation=angle;
            }
        }
    }
    Connections{
        target: xpider_center_
        onXpiderStarted:{
            for(var i=0;i<max_xpider_num;++i){
                var index = parseInt(max_xpider_num*Math.random(i))
                var xpider = createXpider(index)
            }
            console.log("xpdier queue intialized:",xpider_queue_.length);
        }

        onXpiderStoped:{
            //clearXpiderQueue()
            console.log("xpider queue destroyed ")
        }
    }
}
