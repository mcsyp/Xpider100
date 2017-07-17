import QtQuick 2.0

Item {
    property var cross_img_scale: 0.1
    property var cross_id:0
    property var index_:0
    Text{
        id:id_text
        anchors.centerIn: parent
        text:cross_id
        font.pixelSize: 15
        color: "#222222"
    }
    Text{
        anchors.horizontalCenter: parent.horizontalCenter
        y:id_text.y+id_text.contentHeight+2
        text:index_
        font.pointSize: 8
        color: "#101010"
    }
}
