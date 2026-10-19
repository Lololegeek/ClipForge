import QtQuick
import QtQuick.Controls
import "../theme"

Button {
    id: control
    property bool primary: false
    property bool danger: false
    implicitHeight: 40
    implicitWidth: Math.max(88, contentItem.implicitWidth + 30)
    padding: 12
    hoverEnabled: true
    font.family: Theme.font
    font.pixelSize: 14
    contentItem: Text {
        text: control.text
        font: control.font
        color: !control.enabled ? Theme.secondary : (control.primary || control.danger) ? "#101119" : Theme.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    HoverHandler {
        cursorShape: control.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
    background: Rectangle {
        radius: 7
        color: !control.enabled ? Theme.surface : control.danger ? (control.down ? "#e08e97" : control.hovered ? "#ffc4ca" : Theme.danger) : control.primary ? (control.down ? "#7777e0" : control.hovered ? "#a3a3ff" : Theme.accent) : (control.down ? Theme.accentSurface : control.hovered ? Theme.raised : Theme.surface)
        border.color: control.activeFocus ? Theme.accent : "transparent"
        border.width: control.activeFocus ? 2 : 0
        opacity: control.enabled ? 1 : 0.55
    }
}
