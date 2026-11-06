import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    id: page
    spacing: 18
    property bool showOthers: false

    function profileOf(exe) {
        return appController.config.profiles ? appController.config.profiles[exe.toLowerCase()] : undefined;
    }
    function summary(p) {
        if (!p)
            return "";
        let parts = [];
        if (p.width && p.height)
            parts.push(p.width + "×" + p.height);
        if (p.fps)
            parts.push(p.fps + " FPS");
        if (p.replaySeconds)
            parts.push("replay " + p.replaySeconds + " s");
        if (p.bitrate)
            parts.push(Math.round(p.bitrate / 1000) + " Mb/s");
        return parts.join("  ·  ");
    }
    readonly property var shown: {
        const all = appController.gameList;
        let out = [];
        for (let i = 0; i < all.length; ++i)
            if (all[i].game || showOthers)
                out.push(all[i]);
        return out;
    }
    readonly property int hiddenCount: appController.gameList.length - shown.length

    GameDialog {
        id: gameDialog
        objectName: "gameDialog"
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Jeux"
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
        text: "1. Lance ton jeu   ·   2. Clique sur « Actualiser »   ·   3. Clique sur « Ajouter » et choisis tes réglages
Le replay démarrera tout seul à chaque lancement du jeu."
        wrapMode: Text.Wrap
        color: Theme.secondary
        lineHeight: 1.25
    }
    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        model: page.shown
        spacing: 6
        ScrollBar.vertical: ScrollBar {}
        delegate: Rectangle {
            required property var modelData
            readonly property var profile: page.profileOf(modelData.executable)
            width: ListView.view.width
            height: profile ? 92 : 68
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
                    spacing: 3
                    RowLayout {
                        spacing: 8
                        Text {
                            Layout.fillWidth: true
                            text: modelData.title
                            color: Theme.text
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            elide: Text.ElideRight
                        }
                        Rectangle {
                            visible: modelData.running
                            width: runLabel.implicitWidth + 14
                            height: 20
                            radius: 10
                            color: Theme.accentSurface
                            Text {
                                id: runLabel
                                anchors.centerIn: parent
                                text: "en cours"
                                color: Theme.accentHover
                                font.pixelSize: 11
                            }
                        }
                    }
                    Text {
                        visible: !profile
                        Layout.fillWidth: true
                        text: modelData.executable
                        color: Theme.secondary
                        font.pixelSize: 12
                        elide: Text.ElideMiddle
                    }
                    Text {
                        visible: !!profile
                        Layout.fillWidth: true
                        text: "Ajouté  ·  " + page.summary(profile)
                        color: Theme.success
                        font.pixelSize: 13
                        elide: Text.ElideRight
                    }
                    Text {
                        visible: !!profile
                        Layout.fillWidth: true
                        text: "Clique sur « Réglages » pour changer la qualité, les FPS ou la durée du replay."
                        color: Theme.secondary
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
                ActionButton {
                    visible: !!profile
                    text: "Réglages"
                    onClicked: gameDialog.openFor(modelData.executable, modelData.title)
                }
                ActionButton {
                    visible: !!profile
                    text: "Retirer"
                    onClicked: appController.removeProfile(modelData.executable)
                }
                ActionButton {
                    visible: !profile
                    primary: true
                    text: "Ajouter"
                    onClicked: gameDialog.openFor(modelData.executable, modelData.title)
                }
            }
        }
    }
    Text {
        visible: page.shown.length === 0
        Layout.fillWidth: true
        text: "Aucun jeu détecté. Lance ton jeu, puis clique sur Actualiser."
        color: Theme.secondary
        wrapMode: Text.Wrap
    }
    ActionButton {
        visible: page.showOthers || page.hiddenCount > 0
        text: page.showOthers ? "Masquer les autres applications" : "Afficher les autres applications (" + page.hiddenCount + ")"
        onClicked: page.showOthers = !page.showOthers
    }
    Component.onCompleted: appController.refreshWindows()
    Connections {
        target: appController
        function onConfigChanged() {
            appController.refreshWindows();
        }
    }
}
