import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    spacing: 18
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Jeux et applications"
            color: Theme.text
            font.pixelSize: 28
            font.weight: Font.DemiBold
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Actualiser"
            onClicked: appController.refreshWindows()
        }
    }
    Text {
        Layout.fillWidth: true
        text: "Seuls les exécutables auxquels tu attribues un profil déclenchent la capture automatique. Configure d'abord la source dans Paramètres, puis mémorise le profil ci-dessous."
        wrapMode: Text.Wrap
        color: Theme.secondary
    }
    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        model: appController.games
        spacing: 4
        ScrollBar.vertical: ScrollBar {}
        delegate: Rectangle {
            required property var modelData
            width: ListView.view.width
            height: 92
            color: Theme.surface
            radius: 8
            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 16
                ColumnLayout {
                    Layout.fillWidth: true
                    Text {
                        Layout.fillWidth: true
                        text: modelData.title
                        color: Theme.text
                        font.pixelSize: 15
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: modelData.executable
                        color: Theme.secondary
                        font.pixelSize: 11
                        elide: Text.ElideMiddle
                    }
                }
                ActionButton {
                    text: appController.config.profiles[modelData.executable.toLowerCase()] ? "Retirer le profil" : "Mémoriser ce profil"
                    onClicked: appController.config.profiles[modelData.executable.toLowerCase()] ? appController.removeProfile(modelData.executable) : appController.saveProfile(modelData.executable, modelData.title)
                }
            }
        }
    }
    Text {
        text: "La présence d'une fenêtre ne garantit pas sa compatibilité avec la capture."
        color: Theme.secondary
        font.pixelSize: 12
    }
    Component.onCompleted: appController.refreshWindows()
}
