import QtQuick 2.0

Item {
    id:xpider_item_
    property var dev_id:0xff
    property var xpider_img_target_scale_:0.5
    property var selected_: false
    property var xpider_selected_scale_:0.35
    property var world_x: 0
    property var world_y: 0
    Image{
        id:xpider_img_
        source:"images/spider.png"
        anchors.centerIn: parent
        scale:xpider_img_target_scale_
    }
    Image{
        id:xpider_selected_
        source:"images/angle.png"
        anchors.centerIn: parent
        visible: selected_
        scale:xpider_selected_scale_
    }

    Text{
        id: xpider_id_txt_
        anchors.centerIn: parent
        text:dev_id
        font.pixelSize: 12
    }

    SpringAnimation{
        id:animate_scale
        target:xpider_img_
        property: "scale"
        spring: 3.0
        damping: 0.2
        epsilon: 0.05
        from:0.05
        to:xpider_img_target_scale_
        duration:50
    }
    SpringAnimation{
        id:frame_scale_
        target:xpider_selected_
        property: "scale"
        spring: 3.0
        damping: 0.2
        epsilon: 0.05
        from:0.05
        to:xpider_selected_scale_
        duration:30
    }

    RotationAnimation{
        id:frame_rotate_
        target:xpider_selected_
        loops:Animation.Infinite
        duration:3000
        from:0
        to:360
    }
    function setSelected(pressed){
        selected_ = pressed;
        if(selected_){
            animate_scale.target=xpider_img_;
            frame_scale_.target = xpider_selected_
            frame_rotate_.running=true
            animate_scale.running=true;
            frame_scale_.running=true;
        }
    }
/*
    MouseArea{
        anchors.fill:xpider_img_
        onClicked: {
            //setSelected(!selected_)
            playground.childrenClicked(xpider_item_.dev_id);
        }
    }*/

}
