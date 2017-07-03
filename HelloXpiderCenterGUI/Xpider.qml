import QtQuick 2.0

Item {
    id:xpider_item_
    property var dev_id:0xff
    property var xpider_img_target_scale:0.25
    Image{
        id:xpider_img_
        source:"images/xpider_icon.png"
        anchors.centerIn: parent
        scale:xpider_img_target_scale
    }
    Text{
        id: xpider_id_txt_
        anchors.centerIn: parent
        text:dev_id
        font.pixelSize: 8
    }
    function setXpdierID(id){
        xpider_id_txt_.text = id;
    }

    SpringAnimation{
        id:frame_scale_
        target:xpider_img_
        property: "scale"
        spring: 3.0
        damping: 0.2
        epsilon: 0.05
        from:0.05
        to:xpider_img_target_scale
        duration:200
    }
    function show(){
        xpider_item_.visible=true;
        frame_scale_.target=xpider_img_;
        frame_scale_.running=true;
    }
}
