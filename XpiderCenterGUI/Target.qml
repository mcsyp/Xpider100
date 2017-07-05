import QtQuick 2.0

Item {
    id: target_;
    property var dev_id:0
    property var target_img_scale_: 0.25
    Image{
        id: target_img
        source:"images/target.png"
        anchors.centerIn: parent
        scale: target_img_scale_
    }
    Text{
        text:dev_id
        anchors.centerIn: parent
        font.pixelSize: 10
    }


    ParallelAnimation{
        id:animation_seq

        SpringAnimation{
            id:animate_scale
            target:target_img
            property: "scale"
            spring: 3.0
            damping: 0.1
            epsilon: 0.05
            from:0.02
            to:target_img_scale_
            duration:100
        }
    }

    function show(){
        visible = true;
        //animation_number.running=true
        //animate_scale.running = true;
        animation_seq.running=true
    }
}
