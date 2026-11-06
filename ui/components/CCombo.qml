import QtQuick
import QtQuick.Controls
import "../theme"

ComboBox {
    id: control
    implicitHeight: 40
    implicitWidth: 160
    leftPadding: 12
    rightPadding: 32
    font.family: Theme.font
    font.pixelSize: 14
    contentItem: Text {
        text: control.displayText
        font: control.font
        color: control.enabled ? Theme.text : Theme.secondary
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: Theme.radiusSm
        color: control.hovered ? Theme.raised : Theme.surface
        border.color: control.activeFocus ? Theme.accent : Theme.line
        border.width: control.activeFocus ? 2 : 1
        opacity: control.enabled ? 1 : 0.5
    }
    indicator: Canvas {
        x: control.width - width - 12
        y: (control.height - height) / 2
        width: 12
        height: 8
        rotation: control.popup.visible ? 180 : 0
        Behavior on rotation { NumberAnimation { duration: 160; easing.type: Easing.OutCubic } }
        property color tone: control.hovered || control.popup.visible ? Theme.text : Theme.secondary
        onToneChanged: requestPaint()
        onPaint: {
            const c = getContext("2d");
            c.reset();
            c.strokeStyle = tone;
            c.lineWidth = 2;
            c.lineCap = "round";
            c.lineJoin = "round";
            c.beginPath();
            c.moveTo(1.5, 1.5);
            c.lineTo(6, 6);
            c.lineTo(10.5, 1.5);
            c.stroke();
        }
    }
    delegate: ItemDelegate {
        required property int index
        property var rowData: control.model[index]
        width: control.width - 12
        height: 38
        text: control.textAt(index)
        enabled: !rowData || rowData.available !== false
        highlighted: control.highlightedIndex === index
        contentItem: Text {
            text: parent.text
            color: !parent.enabled ? Theme.secondary : (control.currentIndex === parent.index ? Theme.accentHover : Theme.text)
            font: control.font
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            color: parent.highlighted ? Theme.accentSurface : "transparent"
            radius: 5
        }
    }
    popup: Popup {
        y: control.height + 5
        width: control.width
        padding: 6
        implicitHeight: contentItem.implicitHeight + 12
        background: Rectangle {
            color: Theme.raised
            radius: 8
            border.color: Theme.line
        }
        contentItem: ListView {
            clip: true
            implicitHeight: Math.min(contentHeight, 300)
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }
}
