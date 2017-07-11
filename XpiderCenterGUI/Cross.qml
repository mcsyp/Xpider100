import QtQuick 2.0

Item {
    property var cross_img_scale: 0.1
    property var cross_id:0
    Text{
        anchors.centerIn: parent
        text:cross_id
        font.pixelSize: 15
        color: "#222222"
    }
}
