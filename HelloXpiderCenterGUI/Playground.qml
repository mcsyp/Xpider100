import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Rectangle {
    id:playground_
    property var xpider_queue_ : new Array()
    property Component my_component_:null;
    property var max_xpider_num:100;

    function createXpider(id){
        var posx,posy;
        var dynamic_comp_;
        if(my_component_==null){
            my_component_ = Qt.createComponent("Xpider.qml");
        }
        if(my_component_.status===Component.Ready){
            var seed= xpider_queue_.length;
            posx = 100+Math.random(seed)*(parent.width-200);
            posy = 100+Math.random(seed)*(parent.height-200);
            dynamic_comp_ = my_component_.createObject(playground_,
                                                       {"x":posx,
                                                        "y":posy,
                                                        "dev_id":id,
                                                        "z":2,
                                                        "visible":false,});
            //console.log("xpider:",id,"x:",posx,"y:",posy,"length:",xpider_queue_.length);
            xpider_queue_.push(dynamic_comp_);
            return dynamic_comp_;
            //canvas_2d_.requestPaint()
        }
        return null;
    }

    function clearXpiderQueue(){
        if(xpider_queue_){
            for(var i=0;i<xpider_queue_.length;++i){
                xpider_queue_[i].destroy();
            }
            xpider_queue_=[];
            //canvas_2d_.requestPaint();
        }
    }

    MouseArea{
        anchors.fill: parent
        onClicked: {
            console.log(xpider_queue_.length)
        }
    }
    Connections{
      target: opti_server_
      onXpiderUpdate:{
            console.log("index:",id," theta:",theta," x:",x," y:",y)
            console.log("xpider queue length is:",xpider_queue_.length)
      }
    }
    Connections{
        target: xpider_center_
        onXpiderStarted:{
            for(var i=0;i<max_xpider_num;++i){
                var index= parseInt(max_xpider_num*Math.random(i))
                var xpider=createXpider(index)
                xpider.show()
            }
            console.log("haha started");
        }

        onXpiderStoped:{
            clearXpiderQueue()
            console.log("haha stopped")
        }
    }
}
