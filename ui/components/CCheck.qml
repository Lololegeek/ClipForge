import QtQuick
import QtQuick.Controls
import "../theme"

CheckBox {
    id: control
    spacing: 10
    implicitHeight: Math.max(28, contentItem.implicitHeight)
    indicator: Rectangle {
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 20
        height: 20
        radius: 6
        color: control.checked ? Theme.accent : (control.hovered ? Theme.raised : "transparent")
        border.width: 1.5
        border.color: control.checked ? Theme.accent : (control.hovered ? Theme.accentHover : Theme.line)
        opacity: control.enabled ? 1 : 0.45
        Behavior on color { ColorAnimation { duration: 120 } }
        Canvas {
            id: tick
            anchors.fill: parent
            property real shown: control.checked ? 1 : 0
            Behavior on shown { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
            onShownChanged: requestPaint()
            onPaint: {
                const c = getContext("2d");
                c.reset();
                if (shown <= 0)
                    return;
                c.strokeStyle = "#14151f";
                c.lineWidth = 2.4;
                c.lineCap = "round";
                c.lineJoin = "round";
                const p = [[5.2, 10.4], [8.6, 13.8], [14.8, 6.6]];
                c.globalAlpha = shown;
                c.beginPath();
                c.moveTo(p[0][0], p[0][1]);
                c.lineTo(p[1][0], p[1][1]);
                if (shown > 0.5)
                    c.lineTo(p[2][0], p[2][1]);
                c.stroke();
            }
        }
    }
    contentItem: Text {
        text: control.text
        color: Theme.text
        opacity: control.enabled ? 1 : 0.5
        wrapMode: Text.Wrap
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
}
