import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"

// Styled confirmation / form dialog. Extra content goes in the default slot.
Popup {
    id: dialog
    property string title: ""
    property string body: ""
    property string acceptText: "Confirmer"
    property string rejectText: "Annuler"
    property bool danger: false
    property bool acceptEnabled: true
    default property alias content: slot.data
    signal accepted

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: Overlay.overlay
    width: Math.min(460, (parent ? parent.width : 460) - 60)
    padding: 26
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 120 }
        NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: 140; easing.type: Easing.OutCubic }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 90 }
    }
    Overlay.modal: Rectangle {
        color: "#b3101119"
    }
    background: Rectangle {
        radius: Theme.radius
        color: Theme.raised
        border.color: Theme.line
    }
    function submit() {
        if (!acceptEnabled)
            return;
        dialog.close();
        dialog.accepted();
    }
    contentItem: ColumnLayout {
        spacing: 14
        Keys.onReturnPressed: dialog.submit()
        Keys.onEnterPressed: dialog.submit()
        Text {
            Layout.fillWidth: true
            text: dialog.title
            color: Theme.text
            font.pixelSize: 19
            font.weight: Font.DemiBold
            wrapMode: Text.Wrap
        }
        Text {
            Layout.fillWidth: true
            visible: dialog.body.length > 0
            text: dialog.body
            color: Theme.secondary
            font.pixelSize: 14
            wrapMode: Text.Wrap
            lineHeight: 1.15
        }
        ColumnLayout {
            id: slot
            Layout.fillWidth: true
            spacing: 10
        }
        RowLayout {
            Layout.topMargin: 6
            Layout.fillWidth: true
            spacing: 10
            Item {
                Layout.fillWidth: true
            }
            ActionButton {
                text: dialog.rejectText
                onClicked: dialog.close()
            }
            ActionButton {
                text: dialog.acceptText
                primary: !dialog.danger
                danger: dialog.danger
                enabled: dialog.acceptEnabled
                onClicked: dialog.submit()
            }
        }
    }
}
