import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    spacing: 22
    Text {
        text: "Capture"
        color: Theme.text
        font.pixelSize: 28
        font.weight: Font.DemiBold
    }
    Text {
        Layout.fillWidth: true
        text: "Le replay garde les dernières secondes. Appuie sur F" + (appController.config.saveKey - 111) + " pour en faire un clip."
        color: Theme.secondary
        wrapMode: Text.Wrap
    }
    Panel {
        Layout.fillWidth: true
        Layout.preferredHeight: 240
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 26
            spacing: 18
            Text {
                text: appController.captureMethod
                color: Theme.text
                font.pixelSize: 21
            }
            Text {
                text: appController.encoder
                color: Theme.secondary
                font.pixelSize: 14
            }
            Text {
                text: appController.config.width + " × " + appController.config.height + " · " + appController.config.fps + " FPS · " + appController.config.codec.toUpperCase() + " · " + appController.config.replaySeconds + " secondes"
                color: Theme.secondary
                font.family: Theme.mono
            }
            RowLayout {
                spacing: 12
                ActionButton {
                    text: appController.state === "Idle" || appController.state === "Error" ? "Activer le replay" : "Arrêter la capture"
                    primary: true
                    enabled: appController.engineReady && ["Saving", "Initializing", "Stopping"].indexOf(appController.state) < 0
                    onClicked: appController.state === "Idle" || appController.state === "Error" ? appController.start() : appController.stop()
                }
            }
        }
    }
    GridLayout {
        columns: 2
        columnSpacing: 40
        rowSpacing: 20
        Layout.fillWidth: true
        Repeater {
            model: [["FPS effectifs", Number(appController.metrics.fps || 0).toFixed(2)], ["Images en retard", appController.metrics.lagged || 0], ["Rendu moyen", Number(appController.metrics.frameMs || 0).toFixed(2) + " ms"], ["Dernière sauvegarde", (appController.metrics.saveMs || 0) + " ms"], ["Mémoire du processus", Math.round(appController.metrics.ramMB || 0) + " Mo"], ["CPU (tous les cœurs)", Number(appController.metrics.cpu || 0).toFixed(1) + " %"]]
            delegate: RowLayout {
                required property var modelData
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: modelData[0]
                    color: Theme.secondary
                }
                Text {
                    text: modelData[1]
                    color: Theme.text
                    font.family: Theme.mono
                }
            }
        }
    }
    Text {
        Layout.fillWidth: true
        text: "La charge GPU et la taille exacte des paquets du tampon ne sont pas exposées ici. La durée affichée indique le temps écoulé, plafonné au réglage ; les images clés et la limite mémoire peuvent réduire la durée sauvegardée."
        color: Theme.secondary
        wrapMode: Text.Wrap
        font.pixelSize: 12
    }
    Item {
        Layout.fillHeight: true
    }
    ActionButton {
        text: "Configurer la capture"
        onClicked: window.page = 5
    }
}
