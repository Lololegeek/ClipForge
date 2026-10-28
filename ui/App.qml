import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "theme"
import "components"

ApplicationWindow {
    id: window
    width: 1320
    height: 840
    minimumWidth: 980
    minimumHeight: 680
    visible: false
    title: "ClipForge"
    color: Theme.canvas
    font.family: Theme.font
    font.pixelSize: 14
    palette.window: Theme.canvas
    palette.windowText: Theme.text
    palette.text: Theme.text
    palette.base: Theme.navigation
    palette.button: Theme.surface
    palette.buttonText: Theme.text
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.canvas
    palette.placeholderText: Theme.secondary
    property int page: 0
    // Editor mode: false shows the clip to watch, true reveals the editing tools.
    property bool editorEditing: false
    // Fullscreen playback hides the sidebar, header and footer.
    property bool immersive: false
    property int visibilityBeforeImmersive: Window.Windowed
    function setImmersive(on) {
        if (on === immersive)
            return;
        if (on)
            visibilityBeforeImmersive = window.visibility;
        immersive = on;
        window.visibility = on ? Window.FullScreen : visibilityBeforeImmersive;
    }
    property var pageNames: ["Accueil", "Mes clips", "Capture", "Éditeur", "Jeux", "Paramètres"]
    function fileUrl(p) {
        return p ? "file:///" + p.replace(/\\/g, "/") : "";
    }
    onClosing: function (close) {
        close.accepted = appController.quitting;
        if (appController.quitting)
            return;
        if (appController.config.closeToTray)
            window.hide();
        else
            appController.quit();
    }
    Connections {
        target: appController
        function onOpenEditor() {
            window.page = 3;
        }
    }
    RowLayout {
        anchors.fill: parent
        spacing: 0
        Rectangle {
            visible: !window.immersive
            Layout.preferredWidth: window.width < 1100 ? 180 : 216
            Layout.fillHeight: true
            color: Theme.navigation
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 8
                RowLayout {
                    Layout.topMargin: 12
                    Layout.leftMargin: 6
                    Layout.bottomMargin: 28
                    spacing: 10
                    Image {
                        source: "qrc:/assets/icons/clipforge-mark.svg"
                        sourceSize.width: 32
                        sourceSize.height: 32
                        Layout.preferredWidth: 32
                        Layout.preferredHeight: 32
                        smooth: true
                    }
                    Text {
                        text: "ClipForge"
                        color: Theme.text
                        font.pixelSize: 20
                        font.weight: Font.DemiBold
                    }
                }
                Repeater {
                    model: window.pageNames
                    delegate: ActionButton {
                        required property string modelData
                        required property int index
                        Layout.fillWidth: true
                        implicitHeight: 42
                        text: modelData
                        background: Rectangle {
                            radius: Theme.radiusSm
                            color: window.page === index ? Theme.accentSurface : parent.hovered ? Theme.surface : "transparent"
                            border.color: parent.activeFocus ? Theme.accent : "transparent"
                            Behavior on color {
                                ColorAnimation {
                                    duration: 150
                                }
                            }
                        }
                        contentItem: RowLayout {
                            spacing: 12
                            Image {
                                Layout.preferredWidth: 18
                                Layout.preferredHeight: 18
                                source: "qrc:/assets/icons/" + ["home", "clips", "record", "edit", "games", "settings"][index] + ".svg"
                            }
                            Text {
                                Layout.fillWidth: true
                                text: modelData
                                color: window.page === index ? Theme.text : Theme.secondary
                                font.pixelSize: 14
                                font.weight: window.page === index ? Font.DemiBold : Font.Normal
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        onClicked: window.page = index
                    }
                }
                Item {
                    Layout.fillHeight: true
                }
                Text {
                    Layout.leftMargin: 12
                    Layout.bottomMargin: 10
                    text: "Version " + appController.version
                    color: Theme.secondary
                    font.pixelSize: 12
                }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            Rectangle {
                visible: !window.immersive
                Layout.fillWidth: true
                Layout.preferredHeight: 64
                color: Theme.canvas
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 30
                    anchors.rightMargin: 30
                    spacing: 10
                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: ["Buffering", "Saving", "Recording"].indexOf(appController.state) >= 0 ? Theme.success : appController.state === "Error" ? Theme.danger : Theme.secondary
                    }
                    Text {
                        text: ({
                                "Idle": "Replay en pause",
                                "Initializing": "Démarrage…",
                                "Buffering": "Replay actif",
                                "Saving": "Sauvegarde du clip…",
                                "Recording": "Enregistrement en cours",
                                "Stopping": "Arrêt en cours…",
                                "Error": "Replay interrompu"
                            })[appController.state] || appController.state
                        color: Theme.text
                        font.pixelSize: 13
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Text {
                        text: appController.currentGame
                        color: Theme.secondary
                        font.pixelSize: 12
                    }
                    ActionButton {
                        text: "Sauvegarder · F" + (appController.config.saveKey - 111)
                        primary: true
                        enabled: ["Buffering", "Saving", "Recording"].indexOf(appController.state) >= 0
                        onClicked: appController.saveReplay()
                    }
                }
            }
            Rectangle {
                visible: !window.immersive
                Layout.fillWidth: true
                height: 1
                color: Theme.line
            }
            Loader {
                id: pageLoader
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: window.immersive ? 0 : 30
                source: "pages/" + ["HomePage.qml", "ClipsPage.qml", "RecordPage.qml", "EditorPage.qml", "GamesPage.qml", "SettingsPage.qml"][window.page]
            }
            Rectangle {
                visible: !window.immersive
                Layout.fillWidth: true
                Layout.preferredHeight: Math.max(40, statusText.implicitHeight + 18)
                color: Theme.navigation
                Text {
                    id: statusText
                    anchors.fill: parent
                    anchors.margins: 9
                    anchors.leftMargin: 30
                    anchors.rightMargin: 30
                    text: appController.message || ("F" + (appController.config.saveKey - 111) + " sauvegarde un clip · Maj+F" + (appController.config.saveKey - 111) + " garde tout le replay")
                    color: Theme.secondary
                    font.pixelSize: 12
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}
