import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import "../theme"

// First-run guide covering the whole window: setup of the main settings, then a short tour of the app.
// Choices are kept locally and written to the configuration once, when the guide is finished.
Rectangle {
    id: root
    signal finished

    property int step: 0
    readonly property var cfg: appController.config

    // ----- choices (initialised from the current configuration) -----
    property int modeIndex: Math.max(0, ["display", "window", "game"].indexOf(cfg.mode))
    property string targetValue: cfg.target || ""
    property int qualityIndex: {
        for (var i = 0; i < presets.length; ++i)
            if (presets[i].w === cfg.width && presets[i].h === cfg.height && presets[i].fps === cfg.fps && presets[i].bitrate === cfg.bitrate)
                return i;
        return -1;
    }
    property int secondsIndex: {
        var i = replayChoices.indexOf(cfg.replaySeconds);
        return i >= 0 ? i : 1;
    }
    property bool systemAudio: cfg.audio !== "disabled"
    property string micValue: cfg.microphone || "disabled"
    property int saveCode: cfg.saveKey
    property int markerCode: cfg.markerKey
    property int cleanupIndex: {
        var i = cleanupChoices.indexOf(cfg.cleanupGB);
        return i >= 0 ? i : 0;
    }
    property bool launchWithWindows: appController.autostart
    property bool closeToTray: cfg.closeToTray
    property bool replayOnLaunch: cfg.alwaysReplay

    readonly property var presets: [
        {
            name: "Léger",
            detail: "720p · 30 images/s · petits fichiers, pour un PC modeste",
            w: 1280,
            h: 720,
            fps: 30,
            bitrate: 6000
        },
        {
            name: "Équilibré",
            detail: "1080p · 60 images/s · le bon choix pour la plupart des PC",
            w: 1920,
            h: 1080,
            fps: 60,
            bitrate: 16000
        },
        {
            name: "Maximum",
            detail: "1440p · 60 images/s · très net, fichiers lourds, PC puissant",
            w: 2560,
            h: 1440,
            fps: 60,
            bitrate: 30000
        }
    ]
    readonly property var replayChoices: [30, 60, 120, 300]
    readonly property var replayLabels: ["30 secondes", "1 minute", "2 minutes", "5 minutes"]
    readonly property var cleanupChoices: [0, 20, 50, 100]
    readonly property var cleanupLabels: ["Illimité : je garde tout", "20 Go maximum", "50 Go maximum", "100 Go maximum"]
    readonly property var modeNames: ["Écran entier", "Fenêtre", "Jeu"]
    readonly property var micRows: [
        {
            name: "Désactivé",
            value: "disabled"
        },
        {
            name: "Micro par défaut de Windows",
            value: "default"
        }
    ].concat(appController.microphones.filter(function (m) {
        return m.value !== "default";
    }))
    readonly property var targetRows: {
        var kind = ["display", "window", "game"][modeIndex];
        var rows = appController.targets.filter(function (t) {
            return t.mode === (kind === "game" ? "window" : kind);
        });
        if (kind === "game")
            rows.unshift({
                mode: "game",
                name: "Jeu en plein écran (détection automatique)",
                value: ""
            });
        return rows;
    }
    readonly property string saveKeyName: appController.keyName(saveCode)
    readonly property string fullKeyName: appController.keyName(saveCode | 0x40000)
    readonly property string markerKeyName: markerCode > 0 ? appController.keyName(markerCode) : ""
    readonly property bool keyConflict: markerCode > 0 && (markerCode === saveCode || markerCode === (saveCode | 0x40000))
    readonly property string targetName: {
        for (var i = 0; i < targetRows.length; ++i)
            if (targetRows[i].value === targetValue)
                return targetRows[i].name;
        return targetRows.length > 0 ? targetRows[0].name : "";
    }
    readonly property string micName: {
        for (var i = 0; i < micRows.length; ++i)
            if (micRows[i].value === micValue)
                return micRows[i].name;
        return "Désactivé";
    }

    readonly property var steps: [
        {
            badge: "qrc:/assets/icons/clipforge-mark.svg",
            title: "Bienvenue sur ClipForge",
            subtitle: "Ne rate plus jamais un moment de jeu. On règle l'essentiel ensemble en moins d'une minute, puis je te montre comment ça marche."
        },
        {
            badge: "🖥",
            title: "Que veux-tu capturer ?",
            subtitle: "ClipForge garde en mémoire les dernières secondes de cette source. Écran entier est le plus simple et marche avec presque tout."
        },
        {
            badge: "✨",
            title: "Qualité et durée du replay",
            subtitle: "Plus c'est net, plus les fichiers sont lourds. Tu pourras tout ajuster plus tard dans les Paramètres."
        },
        {
            badge: "🔊",
            title: "Son et micro",
            subtitle: "Choisis ce qui est enregistré dans tes clips. Le jeu et ton micro sont gardés sur des pistes séparées, réglables dans l'éditeur."
        },
        {
            badge: "⌨",
            title: "Tes raccourcis",
            subtitle: "Ils marchent même quand tu es en plein jeu. Choisis des touches que ton jeu n'utilise pas."
        },
        {
            badge: "📁",
            title: "Où ranger tes clips ?",
            subtitle: "Les clips sont de simples fichiers MP4 que tu retrouves dans l'Explorateur. Tu peux limiter l'espace utilisé."
        },
        {
            badge: "▶",
            title: "Comment ça marche",
            subtitle: "Le replay tourne en continu et ne garde que les dernières secondes. Rien n'est enregistré tant que tu ne sauvegardes pas."
        },
        {
            badge: "🧭",
            title: "Un tour de l'application",
            subtitle: "Le menu à gauche te mène partout. Voici à quoi sert chaque page."
        },
        {
            badge: "✔",
            title: "Tout est prêt",
            subtitle: "Voici ce qui va être appliqué. Tu pourras tout modifier dans Paramètres."
        }
    ]
    readonly property bool last: step === steps.length - 1
    readonly property bool canContinue: !(step === 4 && keyConflict)

    color: Theme.canvas

    function next() {
        if (!canContinue)
            return;
        if (last)
            finish(true);
        else
            step++;
    }
    function targetIndex() {
        for (var i = 0; i < targetRows.length; ++i)
            if (targetRows[i].value === targetValue)
                return i;
        return targetRows.length > 0 ? 0 : -1;
    }
    function finish(applyChoices) {
        var c = Object.assign({}, appController.config);
        c.onboarded = true;
        if (applyChoices) {
            c.mode = ["display", "window", "game"][modeIndex];
            c.target = targetValue;
            if (qualityIndex >= 0) {
                var q = presets[qualityIndex];
                c.width = q.w;
                c.height = q.h;
                c.fps = q.fps;
                c.bitrate = q.bitrate;
            }
            c.replaySeconds = replayChoices[secondsIndex];
            c.audio = systemAudio ? (cfg.audio === "process" ? "process" : "system") : "disabled";
            c.microphone = micValue;
            c.saveKey = saveCode;
            c.markerKey = markerCode;
            c.cleanupGB = cleanupChoices[cleanupIndex];
            c.closeToTray = closeToTray;
            c.alwaysReplay = replayOnLaunch;
            if (launchWithWindows !== appController.autostart)
                appController.setAutostart(launchWithWindows);
        }
        appController.apply(c);
        finished();
    }

    // A selectable option card.
    component Choice: Rectangle {
        id: card
        property string title
        property string detail
        property bool selected: false
        signal picked
        Layout.fillWidth: true
        implicitHeight: 58
        radius: Theme.radiusSm
        color: selected ? Theme.accentSurface : cardHover.hovered ? Theme.raised : Theme.surface
        border.color: selected ? Theme.accent : Theme.line
        border.width: selected ? 2 : 1
        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
        ColumnLayout {
            anchors.fill: parent
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            spacing: 2
            Item {
                Layout.fillHeight: true
            }
            Text {
                text: card.title
                color: Theme.text
                font.pixelSize: 15
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                visible: card.detail !== ""
                text: card.detail
                color: Theme.secondary
                font.pixelSize: 12
                elide: Text.ElideRight
            }
            Item {
                Layout.fillHeight: true
            }
        }
        HoverHandler {
            id: cardHover
            cursorShape: Qt.PointingHandCursor
        }
        TapHandler {
            onTapped: card.picked()
        }
    }
    // A label / value row for the recap.
    component Fact: RowLayout {
        property string label
        property string value
        Layout.fillWidth: true
        spacing: 16
        Text {
            Layout.preferredWidth: 110
            text: parent.label
            color: Theme.secondary
            font.pixelSize: 14
        }
        Text {
            Layout.fillWidth: true
            text: parent.value
            color: Theme.text
            font.pixelSize: 14
            elide: Text.ElideMiddle
        }
    }
    // An icon + title + sentence row for the tour.
    component Tour: RowLayout {
        property string glyph
        property string title
        property string detail
        Layout.fillWidth: true
        spacing: 16
        Rectangle {
            Layout.preferredWidth: 44
            Layout.preferredHeight: 44
            Layout.alignment: Qt.AlignTop
            radius: 12
            color: Theme.accentSurface
            Image {
                anchors.centerIn: parent
                width: 20
                height: 20
                sourceSize: Qt.size(40, 40)
                source: parent.parent.glyph
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Text {
                text: parent.parent.title
                color: Theme.text
                font.pixelSize: 15
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: parent.parent.detail
                color: Theme.secondary
                font.pixelSize: 13
                wrapMode: Text.Wrap
            }
        }
    }

    // Swallow input so nothing behind the overlay reacts.
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
    }
    // Soft glow behind the content.
    Shape {
        anchors.fill: parent
        ShapePath {
            strokeColor: "transparent"
            fillGradient: RadialGradient {
                centerX: root.width / 2
                centerY: root.height * 0.3
                centerRadius: Math.max(root.width, root.height) * 0.55
                focalX: centerX
                focalY: centerY
                GradientStop {
                    position: 0
                    color: "#2e8c8cff"
                }
                GradientStop {
                    position: 1
                    color: "#008c8cff"
                }
            }
            startX: 0
            startY: 0
            PathLine {
                x: root.width
                y: 0
            }
            PathLine {
                x: root.width
                y: root.height
            }
            PathLine {
                x: 0
                y: root.height
            }
            PathLine {
                x: 0
                y: 0
            }
        }
    }
    ActionButton {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 28
        visible: !root.last
        text: "Passer le guide"
        onClicked: root.finish(false)
    }
    Text {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 36
        text: "Étape " + (root.step + 1) + " sur " + root.steps.length
        color: Theme.secondary
        font.pixelSize: 13
    }

    ColumnLayout {
        id: content
        anchors.centerIn: parent
        width: Math.min(640, root.width - 64)
        spacing: 16

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 68
            Layout.preferredHeight: 68
            radius: 20
            color: Theme.accentSurface
            border.color: Theme.accent
            Image {
                anchors.centerIn: parent
                width: 34
                height: 34
                sourceSize: Qt.size(68, 68)
                visible: root.steps[root.step].badge.indexOf("qrc:") === 0
                source: visible ? root.steps[root.step].badge : ""
                smooth: true
            }
            Text {
                anchors.centerIn: parent
                visible: root.steps[root.step].badge.indexOf("qrc:") !== 0
                text: root.steps[root.step].badge
                color: Theme.text
                font.family: "Segoe UI Emoji"
                font.pixelSize: 30
            }
        }
        ColumnLayout {
            id: head
            Layout.fillWidth: true
            spacing: 8
            Text {
                Layout.fillWidth: true
                text: root.steps[root.step].title
                color: Theme.text
                font.pixelSize: 28
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }
            Text {
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                text: root.steps[root.step].subtitle
                color: Theme.secondary
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
                wrapMode: Text.Wrap
            }
        }

        StackLayout {
            id: pages
            Layout.fillWidth: true
            Layout.preferredHeight: 300
            currentIndex: root.step
            onCurrentIndexChanged: fade.restart()
            NumberAnimation {
                id: fade
                target: pages
                property: "opacity"
                from: 0
                to: 1
                duration: 220
                easing.type: Easing.OutCubic
            }

            // 0 · welcome
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 12
                    Tour {
                        glyph: "qrc:/assets/icons/record.svg"
                        title: "Un replay toujours prêt"
                        detail: "Les dernières secondes de ta partie sont gardées en mémoire pendant que tu joues."
                    }
                    Tour {
                        glyph: "qrc:/assets/icons/clips.svg"
                        title: "Un clip d'une touche"
                        detail: "Un truc incroyable vient d'arriver ? Appuie sur ta touche : c'est sauvegardé."
                    }
                    Tour {
                        glyph: "qrc:/assets/icons/edit.svg"
                        title: "Coupe et partage"
                        detail: "Regarde le clip, coupe le meilleur moment et exporte-le pour Discord, YouTube ou en vertical."
                    }
                    Text {
                        Layout.fillWidth: true
                        Layout.topMargin: 6
                        text: "Gratuit, sans compte, sans télémétrie."
                        color: Theme.secondary
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
            // 1 · source
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 10
                    Choice {
                        title: "Écran entier"
                        detail: "Tout ce qui s'affiche sur un écran. Le plus simple, recommandé."
                        selected: root.modeIndex === 0
                        onPicked: root.modeIndex = 0
                    }
                    Choice {
                        title: "Fenêtre"
                        detail: "Une seule application, même si tu en ouvres d'autres à côté."
                        selected: root.modeIndex === 1
                        onPicked: root.modeIndex = 1
                    }
                    Choice {
                        title: "Jeu"
                        detail: "Capture directe d'un jeu en plein écran, avec détection automatique."
                        selected: root.modeIndex === 2
                        onPicked: root.modeIndex = 2
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        spacing: 10
                        CCombo {
                            id: targetBox
                            Layout.fillWidth: true
                            textRole: "name"
                            model: root.targetRows
                            onModelChanged: {
                                currentIndex = root.targetIndex();
                                if (currentIndex >= 0)
                                    root.targetValue = model[currentIndex].value;
                            }
                            onActivated: root.targetValue = model[currentIndex].value
                        }
                        ActionButton {
                            text: "Actualiser"
                            onClicked: appController.refreshWindows()
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: targetBox.count === 0
                        text: root.modeIndex === 0 ? "Aucun écran détecté pour l'instant." : "Aucune fenêtre détectée. Ouvre ton jeu ou ton application, puis clique sur Actualiser."
                        color: Theme.secondary
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }
            }
            // 2 · quality
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 10
                    Repeater {
                        model: root.presets
                        delegate: Choice {
                            required property var modelData
                            required property int index
                            title: modelData.name
                            detail: modelData.detail
                            selected: root.qualityIndex === index
                            onPicked: root.qualityIndex = index
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        spacing: 16
                        Text {
                            Layout.preferredWidth: 150
                            text: "Durée du replay"
                            color: Theme.text
                            font.pixelSize: 14
                        }
                        CCombo {
                            Layout.fillWidth: true
                            model: root.replayLabels
                            currentIndex: root.secondsIndex
                            onActivated: root.secondsIndex = currentIndex
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: root.qualityIndex < 0
                        text: "Tes réglages vidéo actuels sont personnalisés : ils sont conservés tant que tu ne choisis pas un profil."
                        color: Theme.secondary
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }
            }
            // 3 · audio
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 10
                    Choice {
                        title: "Son du jeu et de l'ordinateur"
                        detail: "Tout ce que tu entends dans tes enceintes ou ton casque."
                        selected: root.systemAudio
                        onPicked: root.systemAudio = true
                    }
                    Choice {
                        title: "Pas de son"
                        detail: "Les clips sont des vidéos muettes."
                        selected: !root.systemAudio
                        onPicked: root.systemAudio = false
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        spacing: 16
                        Text {
                            Layout.preferredWidth: 150
                            text: "Microphone"
                            color: Theme.text
                            font.pixelSize: 14
                        }
                        CCombo {
                            id: micBox
                            Layout.fillWidth: true
                            textRole: "name"
                            model: root.micRows
                            onModelChanged: {
                                var found = 0;
                                for (var i = 0; i < model.length; ++i)
                                    if (model[i].value === root.micValue)
                                        found = i;
                                currentIndex = found;
                            }
                            onActivated: root.micValue = model[currentIndex].value
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Ajoute ton micro pour commenter tes parties : tu pourras le couper ou le baisser séparément du jeu dans l'éditeur."
                        color: Theme.secondary
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }
            }
            // 4 · shortcuts
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 12
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16
                        Text {
                            Layout.preferredWidth: 150
                            text: "Sauvegarder un clip"
                            color: Theme.text
                            font.pixelSize: 14
                        }
                        HotkeyField {
                            Layout.fillWidth: true
                            code: root.saveCode
                            allowShift: false
                            hint: "Appuie sur ta touche (sans Maj)…"
                            onEdited: c => root.saveCode = c
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16
                        Text {
                            Layout.preferredWidth: 150
                            text: "Poser un marqueur"
                            color: Theme.text
                            font.pixelSize: 14
                        }
                        HotkeyField {
                            Layout.fillWidth: true
                            code: root.markerCode
                            optional: true
                            onEdited: c => root.markerCode = c
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: root.keyConflict
                        text: "Le marqueur doit utiliser une autre touche que la sauvegarde."
                        color: Theme.danger
                        font.pixelSize: 13
                        wrapMode: Text.Wrap
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.topMargin: 6
                        implicitHeight: keysCol.implicitHeight + 28
                        radius: Theme.radiusSm
                        color: Theme.surface
                        border.color: Theme.line
                        ColumnLayout {
                            id: keysCol
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8
                            Fact {
                                label: root.saveKeyName
                                value: "Sauvegarde les dernières secondes en un clip"
                            }
                            Fact {
                                label: root.fullKeyName
                                value: "Garde tout le replay d'un coup"
                            }
                            Fact {
                                visible: root.markerCode > 0
                                label: root.markerKeyName
                                value: "Pose un marqueur sur le moment, visible dans l'éditeur"
                            }
                        }
                    }
                }
            }
            // 5 · folder
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 12
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 48
                        radius: Theme.radiusSm
                        color: Theme.surface
                        border.color: Theme.line
                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            text: appController.config.clips
                            color: Theme.text
                            font.family: Theme.mono
                            font.pixelSize: 13
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideMiddle
                        }
                    }
                    RowLayout {
                        spacing: 10
                        ActionButton {
                            text: "Changer de dossier"
                            onClicked: appController.chooseClipsFolder()
                        }
                        ActionButton {
                            text: "Ouvrir le dossier"
                            onClicked: appController.reveal("")
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.topMargin: 8
                        spacing: 16
                        Text {
                            Layout.preferredWidth: 150
                            text: "Espace des clips"
                            color: Theme.text
                            font.pixelSize: 14
                        }
                        CCombo {
                            Layout.fillWidth: true
                            model: root.cleanupLabels
                            currentIndex: root.cleanupIndex
                            onActivated: root.cleanupIndex = currentIndex
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.cleanupIndex === 0 ? "Aucun clip n'est supprimé automatiquement." : "Au-delà de cette limite, les clips les plus anciens vont dans la corbeille Windows. Tes favoris ne sont jamais supprimés."
                        color: Theme.secondary
                        font.pixelSize: 12
                        wrapMode: Text.Wrap
                    }
                }
            }
            // 6 · how it works
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 14
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        Repeater {
                            model: [
                                {
                                    n: "1",
                                    t: "Tu joues",
                                    d: "Le replay tourne en tâche de fond et garde les " + root.replayChoices[root.secondsIndex] + " dernières secondes."
                                },
                                {
                                    n: "2",
                                    t: "Tu appuies sur " + root.saveKeyName,
                                    d: "Au bon moment, ou juste après. Une notification confirme la sauvegarde."
                                },
                                {
                                    n: "3",
                                    t: "Le clip est prêt",
                                    d: "Il apparaît dans Mes clips, prêt à être regardé, coupé et partagé."
                                }
                            ]
                            delegate: Rectangle {
                                required property var modelData
                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                implicitHeight: 168
                                radius: Theme.radius
                                color: Theme.surface
                                border.color: Theme.line
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 8
                                    Rectangle {
                                        Layout.preferredWidth: 30
                                        Layout.preferredHeight: 30
                                        radius: 15
                                        color: Theme.accent
                                        Text {
                                            anchors.centerIn: parent
                                            text: modelData.n
                                            color: Theme.canvas
                                            font.pixelSize: 15
                                            font.weight: Font.Bold
                                        }
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.t
                                        color: Theme.text
                                        font.pixelSize: 15
                                        font.weight: Font.DemiBold
                                        wrapMode: Text.Wrap
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.d
                                        color: Theme.secondary
                                        font.pixelSize: 13
                                        wrapMode: Text.Wrap
                                    }
                                    Item {
                                        Layout.fillHeight: true
                                    }
                                }
                            }
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Le replay démarre avec ClipForge. Son état est toujours visible en haut de la fenêtre, et le bouton « Sauvegarder » fait la même chose que ta touche."
                        color: Theme.secondary
                        font.pixelSize: 13
                        wrapMode: Text.Wrap
                    }
                }
            }
            // 7 · tour
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 12
                    Tour {
                        glyph: "qrc:/assets/icons/home.svg"
                        title: "Accueil et Capture"
                        detail: "L'état du replay, ses statistiques et tes derniers clips d'un coup d'œil."
                    }
                    Tour {
                        glyph: "qrc:/assets/icons/clips.svg"
                        title: "Mes clips"
                        detail: "Recherche, favoris, tags, import de vidéos. Clic droit pour renommer ou ouvrir le dossier."
                    }
                    Tour {
                        glyph: "qrc:/assets/icons/edit.svg"
                        title: "Éditeur"
                        detail: "Regarder (F pour le plein écran) ou Modifier : couper, vitesse, recadrage, puis export Discord, YouTube, vertical ou GIF."
                    }
                    Tour {
                        glyph: "qrc:/assets/icons/games.svg"
                        title: "Jeux"
                        detail: "Ajoute un jeu pour que le replay démarre tout seul quand tu le lances."
                    }
                    Tour {
                        glyph: "qrc:/assets/icons/settings.svg"
                        title: "Paramètres"
                        detail: "Tout ce que tu viens de régler, et plus encore. Tu peux aussi exporter ta configuration."
                    }
                }
            }
            // 8 · recap
            Item {
                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    spacing: 12
                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: recap.implicitHeight + 28
                        radius: Theme.radiusSm
                        color: Theme.surface
                        border.color: Theme.line
                        ColumnLayout {
                            id: recap
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 8
                            Fact {
                                label: "Capture"
                                value: root.modeNames[root.modeIndex] + (root.targetName !== "" ? " · " + root.targetName : "")
                            }
                            Fact {
                                label: "Qualité"
                                value: (root.qualityIndex >= 0 ? root.presets[root.qualityIndex].name : "Réglages actuels") + " · replay de " + root.replayLabels[root.secondsIndex]
                            }
                            Fact {
                                label: "Son"
                                value: (root.systemAudio ? "Ordinateur" : "Aucun son") + " · micro : " + root.micName
                            }
                            Fact {
                                label: "Raccourcis"
                                value: "Sauvegarde " + root.saveKeyName + (root.markerCode > 0 ?" · marqueur " + root.markerKeyName : "")
                            }
                            Fact {
                                label: "Dossier"
                                value: appController.config.clips
                            }
                        }
                    }
                    CCheck {
                        text: "Lancer le replay au démarrage de ClipForge"
                        checked: root.replayOnLaunch
                        onToggled: root.replayOnLaunch = checked
                    }
                    CCheck {
                        text: "Garder ClipForge actif dans la zone de notification à la fermeture"
                        checked: root.closeToTray
                        onToggled: root.closeToTray = checked
                    }
                    CCheck {
                        text: "Lancer ClipForge au démarrage de Windows"
                        checked: root.launchWithWindows
                        onToggled: root.launchWithWindows = checked
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 6
            Repeater {
                model: root.steps.length
                delegate: Rectangle {
                    required property int index
                    width: root.step === index ? 24 : 8
                    height: 6
                    radius: 3
                    color: index <= root.step ? Theme.accent : Theme.line
                    Behavior on width {
                        NumberAnimation {
                            duration: 200
                            easing.type: Easing.OutCubic
                        }
                    }
                    Behavior on color {
                        ColorAnimation {
                            duration: 200
                        }
                    }
                }
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12
            ActionButton {
                visible: root.step > 0
                text: "Retour"
                onClicked: root.step--
            }
            ActionButton {
                primary: true
                implicitWidth: 180
                implicitHeight: 46
                enabled: root.canContinue
                text: root.last ? "Terminer et lancer" : root.step === 0 ? "C'est parti" : "Suivant"
                onClicked: root.next()
            }
        }
    }
}
