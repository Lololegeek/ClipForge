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
        text: "Ajoute un jeu pour que le replay démarre tout seul quand tu le lances. La source réglée dans Paramètres est mémorisée avec le jeu."
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
            height: 72
            color: rowHover.hovered ? Theme.raised : Theme.surface
            radius: Theme.radiusSm
            HoverHandler {
                id: rowHover
            }
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
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: modelData.executable
                        color: Theme.secondary
                        font.pixelSize: 12
                        elide: Text.ElideMiddle
                    }
                }
                ActionButton {
                    primary: !appController.config.profiles[modelData.executable.toLowerCase()]
                    text: appController.config.profiles[modelData.executable.toLowerCase()] ? "Retirer" : "Ajouter"
                    onClicked: appController.config.profiles[modelData.executable.toLowerCase()] ? appController.removeProfile(modelData.executable) : appController.saveProfile(modelData.executable, modelData.title)
                }
            }
        }
    }
    Text {
        visible: appController.games.length === 0
        Layout.fillWidth: true
        text: "Aucun jeu ouvert. Lance ton jeu, puis clique sur Actualiser."
        color: Theme.secondary
        wrapMode: Text.Wrap
    }
    Component.onCompleted: appController.refreshWindows()
}
