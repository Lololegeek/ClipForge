import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    spacing: 24
    Text {
        text: "Accueil"
        color: Theme.text
        font.pixelSize: 28
        font.weight: Font.DemiBold
    }
    Panel {
        Layout.fillWidth: true
        Layout.preferredHeight: 200
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: "Replay"
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
                Text {
                    visible: (appController.metrics.encoderLoad || 0) > 2
                    text: "  Images perdues : baisse la qualité"
                    color: Theme.danger
                    font.pixelSize: 12
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
                    text: Theme.sourceLabel(appController.config.mode) + " · " + appController.config.height + "p " + appController.config.fps + " FPS"
                    color: Theme.secondary
                    font.pixelSize: 13
                    elide: Text.ElideRight
                }
                ActionButton {
                    text: appController.state === "Idle" || appController.state === "Error" ? "Activer le replay" : "Mettre en pause"
                    primary: true
                    enabled: appController.engineReady && ["Initializing", "Stopping", "Saving"].indexOf(appController.state) < 0
                    onClicked: appController.state === "Idle" || appController.state === "Error" ? appController.start() : appController.stop()
                }
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 32
        Repeater {
            model: [["Clips", clipLibrary.count], ["Images par seconde", Number(appController.metrics.fps || 0).toFixed(0)], ["Images perdues", Number(appController.metrics.encoderLoad || 0).toFixed(1) + " %"], ["Mémoire du replay", Math.round(appController.metrics.bufferMB || 0) + " Mo"], ["Espace libre", Number(appController.metrics.diskGB || 0).toFixed(1) + " Go"]]
            delegate: Column {
                required property var modelData
                spacing: 4
                Text {
                    text: modelData[1]
                    color: Theme.text
                    font.pixelSize: 18
                    font.weight: Font.DemiBold
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
        Rectangle {
            visible: appController.micMuted && appController.config.microphone !== "disabled"
            implicitWidth: micLabel.implicitWidth + 20
            implicitHeight: 28
            radius: height / 2
            color: Theme.raised
            Text {
                id: micLabel
                anchors.centerIn: parent
                text: appController.config.pushToTalk ? "Micro : maintiens la touche pour parler" : "Micro coupé"
                color: Theme.danger
                font.pixelSize: 12
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Derniers clips"
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
        text: "Aucun clip pour l'instant. Active le replay, puis appuie sur F" + (appController.config.saveKey - 111) + " pendant une partie : le clip apparaîtra ici."
        color: Theme.secondary
        wrapMode: Text.Wrap
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
                radius: Theme.radiusSm
            }
            contentItem: RowLayout {
                spacing: 16
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
                    text: Theme.duration(duration)
                    color: Theme.secondary
                    font.family: Theme.mono
                }
            }
            onClicked: if (!missing) {
                window.editorEditing = false;
                appController.selectClip(index);
            }
        }
    }
}
