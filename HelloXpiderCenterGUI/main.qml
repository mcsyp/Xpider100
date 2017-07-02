import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    visible: true
    width:640
    height:480
    Playground {
     id:playground_
    }
    onClosing: {
        console.debug("called here?")
        xpider_center_.StopConnection()
    }
    Connections{
        target:xpider_opti_
        onXpiderUpdate:{
           console.log("index:",index," theta:",theta," x:",x," y:",y)
        }
    }
}
