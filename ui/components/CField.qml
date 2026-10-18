import QtQuick
import QtQuick.Controls
import "../theme"

TextField {
    id: control
    implicitHeight: 40
    leftPadding: 12
    rightPadding: 12
    color: Theme.text
    placeholderTextColor: Theme.secondary
    selectionColor: Theme.accent
    selectedTextColor: Theme.canvas
    font.family: Theme.font
    font.pixelSize: 14
    background: Rectangle {
        radius: 7
        color: Theme.navigation
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? Theme.accent : Theme.line
        opacity: control.enabled ? 1 : 0.5
    }
}
