import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    spacing: 22
    Text {
        text: "Garde le meilleur."
        color: Theme.text
        font.pixelSize: 30
        font.weight: Font.DemiBold
    }
    Text {
        text: "Le jeu continue. Le moment reste."
        color: Theme.secondary
        font.pixelSize: 14
    }
    Panel {
        Layout.fillWidth: true
        Layout.preferredHeight: 210
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Instant Replay"
                    color: Theme.text
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }
                Item {
                    Layout.fillWidth: true
                }
                Text {
                    text: Math.floor(appController.metrics.bufferSeconds || 0) + " / " + appController.config.replaySeconds + " s"
                    color: Theme.accent
                    font.family: Theme.mono
                    font.pixelSize: 16
                }
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 46
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    height: 4
                    radius: 2
                    color: Theme.line
                    Rectangle {
                        width: parent.width * Math.min(1, (appController.metrics.bufferSeconds || 0) / appController.config.replaySeconds)
                        height: parent.height
                        radius: 2
                        color: Theme.accent
                    }
                }
                Row {
                    anchors.fill: parent
                    Repeater {
                        model: 31
                        Item {
                            width: parent.width / 31
                            height: parent.height
                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: 1
                                height: index % 5 === 0 ? 10 : 5
                                color: Theme.secondary
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 16
                Text {
                    Layout.fillWidth: true
                    text: appController.captureMethod + "\n" + appController.encoder
                    color: Theme.secondary
                    font.pixelSize: 12
                    lineHeight: 1.5
                    elide: Text.ElideRight
                }
                ActionButton {
                    text: appController.state === "Idle" || appController.state === "Error" ? "Activer le replay" : "Arrêter"
                    primary: true
                    enabled: appController.engineReady && ["Initializing", "Stopping", "Saving"].indexOf(appController.state) < 0
                    onClicked: appController.state === "Idle" || appController.state === "Error" ? appController.start() : appController.stop()
                }
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 28
        Repeater {
            model: [["Clips", clipLibrary.count], ["RAM", Math.round(appController.metrics.ramMB || 0) + " Mo"], ["Capture", Number(appController.metrics.fps || 0).toFixed(1) + " FPS"], ["Stockage libre", Number(appController.metrics.diskGB || 0).toFixed(1) + " Go"]]
            delegate: Column {
                required property var modelData
                spacing: 6
                Text {
                    text: modelData[1]
                    color: Theme.text
                    font.pixelSize: 19
                    font.family: Theme.mono
                }
                Text {
                    text: modelData[0]
                    color: Theme.secondary
                    font.pixelSize: 12
                }
            }
        }
        Item {
            Layout.fillWidth: true
        }
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Derniers moments"
            color: Theme.text
            font.pixelSize: 20
            font.weight: Font.DemiBold
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Voir mes clips"
            onClicked: window.page = 1
        }
    }
    Text {
        visible: clipLibrary.count === 0
        Layout.fillWidth: true
        text: "Ta bibliothèque commence avec un moment.\nActive le replay, puis appuie sur F8 pour le retrouver ici."
        color: Theme.secondary
        font.pixelSize: 15
        lineHeight: 1.6
    }
    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        spacing: 2
        model: clipLibrary
        delegate: ItemDelegate {
            required property int index
            required property string clipName
            required property real duration
            required property string game
            required property string created
            required property bool missing
            width: ListView.view.width
            height: 60
            background: Rectangle {
                color: parent.hovered ? Theme.surface : "transparent"
                radius: 7
            }
            contentItem: RowLayout {
                Text {
                    Layout.fillWidth: true
                    text: clipName
                    color: Theme.text
                    elide: Text.ElideRight
                }
                Text {
                    text: game
                    color: Theme.secondary
                }
                Text {
                    text: missing ? "Fichier manquant" : Theme.date(created)
                    color: missing ? Theme.danger : Theme.secondary
                    font.pixelSize: 12
                }
                Text {
                    text: Theme.duration(duration) + "   →"
                    color: Theme.secondary
                    font.family: Theme.mono
                }
            }
            onClicked: if (!missing)
                appController.selectClip(index)
        }
    }
}
