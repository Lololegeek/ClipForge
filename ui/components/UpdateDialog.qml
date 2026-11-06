import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"

// Update window: what is new, how much will be downloaded, progress, then restart.
// Opens by itself when an update is found or ready (see App.qml); "Plus tard" hides it for that step.
Popup {
    id: root
    readonly property string st: appController.updateState
    readonly property bool busy: st === "downloading" || st === "checking"
    readonly property bool ready: st === "ready"
    readonly property double mb: appController.updateSizeMB

    function sizeText(v) {
        return v >= 10 ? Math.round(v) + " Mo" : v.toFixed(1).replace(".", ",") + " Mo";
    }

    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape
    anchors.centerIn: Overlay.overlay
    width: Math.min(520, (parent ? parent.width : 520) - 60)
    padding: 0
    enter: Transition {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 160 }
        NumberAnimation { property: "scale"; from: 0.94; to: 1; duration: 220; easing.type: Easing.OutBack; easing.overshoot: 1.2 }
    }
    exit: Transition {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 110 }
    }
    Overlay.modal: Rectangle {
        color: "#c0101119"
    }
    background: Rectangle {
        radius: Theme.radius + 4
        color: Theme.raised
        border.color: Theme.line
        clip: true
    }

    contentItem: ColumnLayout {
        spacing: 0

        // ----- header -----
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 132
            Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                radius: Theme.radius + 3
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0; color: "#3a3a7a" }
                    GradientStop { position: 0.55; color: "#2c2d52" }
                    GradientStop { position: 1; color: Theme.raised }
                }
                // square off the bottom corners so the header blends into the body
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.radius
                    color: Theme.raised
                }
            }
            // soft glow behind the logo
            Rectangle {
                x: 14 // logo tile is at x=30, 64 wide -> centre 62; glow is 96 wide
                y: 18
                width: 96
                height: 96
                radius: 48
                color: Theme.accent
                opacity: 0.18
            }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 30
                anchors.rightMargin: 26
                spacing: 18
                Rectangle {
                    Layout.preferredWidth: 64
                    Layout.preferredHeight: 64
                    radius: 18
                    color: Theme.canvas
                    border.color: "#4d4d8f"
                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/assets/icons/clipforge-mark.svg"
                        sourceSize.width: 38
                        sourceSize.height: 38
                    }
                    // little badge: arrow while there is something to get, check when ready
                    Rectangle {
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: -5
                        width: 24
                        height: 24
                        radius: 12
                        color: root.ready ? Theme.success : Theme.accent
                        border.color: Theme.raised
                        border.width: 3
                        Text {
                            anchors.centerIn: parent
                            text: root.ready ? "✓" : "↓"
                            color: "#101119"
                            font.pixelSize: 12
                            font.bold: true
                        }
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Rectangle {
                        Layout.preferredHeight: 22
                        Layout.preferredWidth: badge.implicitWidth + 18
                        radius: 11
                        color: "#40ffffff"
                        Text {
                            id: badge
                            anchors.centerIn: parent
                            text: root.ready ? "PRÊTE À INSTALLER" : "NOUVELLE VERSION"
                            color: Theme.text
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
                            font.letterSpacing: 1.2
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "ClipForge " + appController.updateVersion
                        color: Theme.text
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Vous utilisez la version " + appController.version
                        color: Theme.secondary
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }
                }
            }
        }

        // ----- body -----
        ColumnLayout {
            Layout.fillWidth: true
            Layout.margins: 26
            Layout.topMargin: 20
            spacing: 14

            Text {
                text: "Nouveautés"
                color: Theme.text
                font.pixelSize: 15
                font.weight: Font.DemiBold
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(220, notes.implicitHeight + 28)
                radius: Theme.radiusSm
                color: Theme.surface
                border.color: Theme.line
                ScrollView {
                    id: scroller
                    anchors.fill: parent
                    anchors.margins: 14
                    clip: true
                    contentWidth: availableWidth
                    Text {
                        id: notes
                        width: scroller.availableWidth
                        text: appController.updateNotes.length ? appController.updateNotes : "Corrections de bugs et améliorations."
                        textFormat: Text.MarkdownText
                        wrapMode: Text.Wrap
                        color: Theme.secondary
                        linkColor: Theme.accent
                        font.pixelSize: 13
                        lineHeight: 1.2
                        onLinkActivated: link => Qt.openUrlExternally(link)
                        HoverHandler {
                            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                        }
                    }
                }
            }

            // size / status line
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Rectangle {
                    visible: root.mb > 0
                    Layout.preferredHeight: 26
                    Layout.preferredWidth: sizeLabel.implicitWidth + 20
                    radius: 13
                    color: Theme.accentSurface
                    Text {
                        id: sizeLabel
                        anchors.centerIn: parent
                        text: (appController.updateDelta || root.st !== "available" ? "" : "Jusqu'à ") + root.sizeText(root.mb)
                        color: Theme.accentHover
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: !appController.updateSelf ? "Version portable : téléchargez la nouvelle version depuis la page de la release."
                        : appController.updateDelta ? "Seuls les fichiers modifiés sont téléchargés."
                        : root.st === "available" ? "Seuls les fichiers modifiés seront téléchargés quand c'est possible."
                        : ""
                    color: Theme.secondary
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }

            // progress
            ColumnLayout {
                Layout.fillWidth: true
                visible: root.st === "downloading" || root.ready
                spacing: 6
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 8
                    radius: 4
                    color: Theme.surface
                    Rectangle {
                        width: parent.width * (root.ready ? 1 : Math.max(0.02, appController.updateProgress))
                        height: parent.height
                        radius: 4
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0; color: root.ready ? Theme.success : Theme.accent }
                            GradientStop { position: 1; color: root.ready ? "#a8ecc9" : Theme.accentHover }
                        }
                        Behavior on width {
                            NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: root.ready ? "Téléchargée. ClipForge va redémarrer tout seul et le replay reprendra."
                        : "Téléchargement… " + Math.round(appController.updateProgress * 100) + " %"
                        + (root.mb > 0 ? "  ·  " + root.sizeText(root.mb * appController.updateProgress) + " / " + root.sizeText(root.mb) : "")
                    color: root.ready ? Theme.success : Theme.secondary
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }

            RowLayout {
                Layout.topMargin: 6
                Layout.fillWidth: true
                spacing: 10
                Item {
                    Layout.fillWidth: true
                }
                ActionButton {
                    text: "Plus tard"
                    onClicked: root.close()
                }
                ActionButton {
                    primary: true
                    enabled: !root.busy
                    text: root.ready ? "Redémarrer maintenant"
                        : root.st === "downloading" ? "Téléchargement…"
                        : !appController.updateSelf ? "Ouvrir la page"
                        : "Mettre à jour"
                    onClicked: appController.installUpdate()
                }
            }
        }
    }
}
