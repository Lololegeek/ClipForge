import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"

// Per-game setup: video settings used only while this game is running.
CDialog {
    id: root
    property string exe: ""
    property string gameName: ""
    property bool editing: false
    readonly property var sizes: [[0, 0], [1280, 720], [1920, 1080], [2560, 1440], [3840, 2160]]
    readonly property var rates: [30, 60, 120, 144]
    readonly property var qualities: [8000, 16000, 30000, 50000]

    // Opens with the game's saved values, or the current global ones for a new game.
    function openFor(executable, name) {
        const p = appController.config.profiles ? appController.config.profiles[executable.toLowerCase()] : undefined;
        const src = p ? p : appController.config;
        exe = executable;
        gameName = name;
        editing = !!p;
        let s = 0;
        for (let i = 0; i < sizes.length; ++i)
            if (sizes[i][0] === src.width && sizes[i][1] === src.height)
                s = i;
        if (!(src.width === 0 && src.height === 0) && s === 0)
            s = 2;
        size.currentIndex = s;
        rate.currentIndex = Math.max(0, rates.indexOf(src.fps));
        let q = 1;
        for (let i = 0; i < qualities.length; ++i)
            if (src.bitrate >= qualities[i])
                q = i;
        quality.currentIndex = q;
        seconds.text = src.replaySeconds ? src.replaySeconds : 60;
        open();
    }

    title: gameName
    body: editing ? "Réglages vidéo utilisés quand ce jeu est ouvert. Tes réglages habituels reviennent à sa fermeture."
                  : "Choisis les réglages vidéo pour ce jeu. Le replay démarrera tout seul quand tu le lances."
    acceptText: editing ? "Enregistrer" : "Ajouter le jeu"
    acceptEnabled: Number(seconds.text) >= 5
    onAccepted: appController.saveProfile(exe, gameName, {
        "width": sizes[size.currentIndex][0],
        "height": sizes[size.currentIndex][1],
        "fps": rates[rate.currentIndex],
        "bitrate": qualities[quality.currentIndex],
        "replaySeconds": Number(seconds.text)
    })

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        columnSpacing: 16
        rowSpacing: 12
        Label {
            text: "Qualité de l'image"
            color: Theme.text
            Layout.preferredWidth: 150
        }
        CCombo {
            id: size
            Layout.fillWidth: true
            model: ["Taille de l'écran", "720p", "1080p", "1440p", "4K"]
        }
        Label {
            text: "Images par seconde"
            color: Theme.text
        }
        CCombo {
            id: rate
            Layout.fillWidth: true
            model: ["30 FPS", "60 FPS", "120 FPS", "144 FPS"]
        }
        Label {
            text: "Netteté"
            color: Theme.text
        }
        CCombo {
            id: quality
            Layout.fillWidth: true
            model: ["Légère (8 Mb/s)", "Standard (16 Mb/s)", "Haute (30 Mb/s)", "Très haute (50 Mb/s)"]
        }
        Label {
            text: "Durée du replay (s)"
            color: Theme.text
        }
        CField {
            id: seconds
            Layout.fillWidth: true
            validator: IntValidator {
                bottom: 0
                top: 600
            }
        }
    }
}
