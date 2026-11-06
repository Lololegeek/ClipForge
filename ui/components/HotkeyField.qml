import QtQuick
import QtQuick.Controls
import "../theme"

// Click, then press the shortcut you want (F-key, or Ctrl/Alt/Maj + any key).
// Escape cancels; Backspace/Delete clears an optional shortcut.
Rectangle {
    id: root
    property int code: 0
    property bool optional: false
    // Shift+save is reserved for "keep the whole buffer", so the save shortcut forbids Shift.
    property bool allowShift: true
    property string hint: ""
    property bool capturing: false
    property string warning: ""
    signal edited(int code)

    implicitHeight: 40
    implicitWidth: 220
    radius: Theme.radiusSm
    color: Theme.navigation
    border.width: capturing || activeFocus ? 2 : 1
    border.color: capturing ? Theme.accent : Theme.line
    activeFocusOnTab: true

    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        elide: Text.ElideRight
        font.family: Theme.font
        font.pixelSize: 14
        color: root.warning ? Theme.danger : root.capturing ? Theme.accent : (root.code ? Theme.text : Theme.secondary)
        text: root.warning ? root.warning : root.capturing ? (root.hint || "Appuyez sur la combinaison…") : appController.keyName(root.code)
    }
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.capturing = true;
            root.forceActiveFocus();
        }
    }
    Timer {
        id: warningTimer
        interval: 2800
        onTriggered: root.warning = ""
    }
    onActiveFocusChanged: if (!activeFocus) {
        capturing = false;
        warning = "";
    }
    Keys.onPressed: event => {
        if (!capturing) {
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Space) {
                capturing = true;
                event.accepted = true;
            }
            return;
        }
        event.accepted = true;
        if (event.key === Qt.Key_Escape) {
            capturing = false;
            return;
        }
        if ((event.key === Qt.Key_Backspace || event.key === Qt.Key_Delete) && optional && event.modifiers === Qt.NoModifier) {
            capturing = false;
            root.edited(0);
            return;
        }
        // A bare modifier press is not a shortcut yet: wait for the real key.
        var c = appController.keyCode(event.key, event.nativeScanCode, event.modifiers);
        if (c === 0 || !appController.keyUsable(c, allowShift)) {
            // Bare modifier presses are just the start of a combination; anything else gets an explanation.
            const modifierOnly = [Qt.Key_Control, Qt.Key_Shift, Qt.Key_Alt, Qt.Key_Meta, Qt.Key_AltGr].indexOf(event.key) >= 0;
            if (!modifierOnly) {
                root.warning = c !== 0 && !allowShift ? "Maj est réservé au clip complet" : "Refusé : une touche F1–F12, ou Ctrl / Alt + une touche";
                warningTimer.restart();
            }
            return;
        }
        capturing = false;
        root.edited(c);
    }
}
