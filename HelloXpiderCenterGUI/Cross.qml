import QtQuick 2.0

Item {
    property var cross_img_scale: 0.1
    Image{
        id: cross_img
        source:"images/cross.png"
        anchors.centerIn: parent
        scale: cross_img_scale
    }
}
