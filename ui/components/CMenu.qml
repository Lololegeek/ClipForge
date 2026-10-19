import QtQuick
import QtQuick.Controls
import "../theme"

Menu {
    id: menu
    padding: 4
    overlap: 2
    delegate: CMenuItem {}
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 90 }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 60 }
    }
    background: Rectangle {
        implicitWidth: 240
        radius: 10
        color: Theme.raised
        border.color: Theme.line
    }
}
