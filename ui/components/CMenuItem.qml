import QtQuick
import QtQuick.Controls
import "../theme"

MenuItem {
    id: item
    property bool danger: false
    implicitHeight: 38
    implicitWidth: 240
    leftPadding: 14
    rightPadding: 14
    hoverEnabled: true
    font.family: Theme.font
    font.pixelSize: 14
    contentItem: Text {
        text: item.text
        font: item.font
        color: !item.enabled ? Theme.secondary : item.danger ? Theme.danger : Theme.text
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        anchors.fill: parent
        anchors.margins: 3
        radius: 6
        color: item.highlighted || item.hovered ? (item.danger ? "#40ffadb5" : Theme.accentSurface) : "transparent"
    }
}
