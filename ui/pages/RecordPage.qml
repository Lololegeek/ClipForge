import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    spacing: 24
    Text {
        text: "Capture"
        color: Theme.text
        font.pixelSize: 28
        font.weight: Font.DemiBold
    }
    Text {
        Layout.fillWidth: true
        text: "Le replay garde les " + appController.config.replaySeconds + " dernières secondes en mémoire. Appuie sur F" + (appController.config.saveKey - 111) + " pour en faire un clip."
        color: Theme.secondary
        wrapMode: Text.Wrap
    }
    Panel {
        Layout.fillWidth: true
        Layout.preferredHeight: 176
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 12
            Text {
                text: Theme.sourceLabel(appController.config.mode)
                color: Theme.text
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }
            Text {
                text: appController.config.width + " × " + appController.config.height + " · " + appController.config.fps + " images par seconde · " + appController.config.replaySeconds + " s de replay"
                color: Theme.secondary
            }
            Item {
                Layout.fillHeight: true
            }
            RowLayout {
                spacing: 12
                ActionButton {
                    text: appController.state === "Idle" || appController.state === "Error" ? "Activer le replay" : "Mettre en pause"
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
            model: [["Images par seconde", Number(appController.metrics.fps || 0).toFixed(1)], ["Images en retard", appController.metrics.lagged || 0], ["Temps par image", Number(appController.metrics.frameMs || 0).toFixed(1) + " ms"], ["Dernière sauvegarde", (appController.metrics.saveMs || 0) + " ms"], ["Mémoire utilisée", Math.round(appController.metrics.ramMB || 0) + " Mo"], ["Processeur", Number(appController.metrics.cpu || 0).toFixed(1) + " %"]]
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
                    font.weight: Font.DemiBold
                }
            }
        }
    }
    Item {
        Layout.fillHeight: true
    }
    ActionButton {
        text: "Réglages de capture"
        onClicked: window.page = 5
    }
}
