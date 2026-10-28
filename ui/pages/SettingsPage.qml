import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../theme"
import "../components"

ColumnLayout {
    id: settings
    spacing: 16
    property var draft: Object.assign({}, appController.config)
    property bool stopped: appController.state === "Idle" || appController.state === "Error" || appController.state === "Buffering" || appController.state === "Recording"
    property string chosenTarget: settings.draft.target || ""
    function rowsForMode() {
        var kind = ["display", "window", "game"][mode.currentIndex]
        var rows = appController.targets.filter(function (t) { return t.mode === (kind === "game" ? "window" : kind) })
        if (kind === "game")
            rows.unshift({ mode: "game", name: "Jeu plein écran · automatique", value: "" })
        return rows
    }
    function restoreTarget() {
        for (var i = 0; i < target.count; ++i)
            if (target.model[i].value === settings.chosenTarget) {
                target.currentIndex = i
                return
            }
        target.currentIndex = target.count > 0 ? 0 : -1
        if (target.currentIndex >= 0)
            settings.chosenTarget = target.model[target.currentIndex].value
    }
    function codecId() { return ["h264", "hevc", "av1"][codec.currentIndex] || "h264" }
    function encoderRows() {
        return [{ name: "Automatique · matériel puis repli", id: "auto", codec: codecId() }].concat(
            appController.encoders.filter(function (e) { return e.codec === codecId() }))
    }
    function restoreEncoder() {
        for (var i = 0; i < encoder.count; ++i)
            if (encoder.model[i].id === settings.draft.encoder) {
                encoder.currentIndex = i
                return
            }
        encoder.currentIndex = 0
    }
    function keyRows(current) {
        var rows = []
        for (var i = 0; i < 24; ++i) {
            var key = 112 + i
            rows.push({ name: "F" + (i + 1), value: key, available: true })
        }
        return rows
    }
    // Optional keys: first row disables the shortcut.
    function optionalKeyRows() {
        return [{ name: "Désactivée", value: 0 }].concat(keyRows())
    }
    function keyIndex(rows, value) {
        for (var i = 0; i < rows.length; ++i)
            if (rows[i].value === value)
                return i
        return 0
    }
    function apply() {
        draft.mode = ["display", "window", "game"][mode.currentIndex];
        draft.target = target.currentIndex >= 0 ? target.model[target.currentIndex].value : "";
        draft.width = Number(widthInput.text);
        draft.height = Number(heightInput.text);
        draft.fps = [30, 60, 120, 144][fps.currentIndex];
        draft.replaySeconds = Number(seconds.text);
        draft.bitrate = Number(bitrate.text);
        draft.memoryMB = Number(memory.text);
        draft.codec = ["h264", "hevc", "av1"][codec.currentIndex];
        draft.encoder = encoder.currentIndex > 0 ? encoder.model[encoder.currentIndex].id : "auto";
        draft.audio = ["disabled", "system", "process"][audio.currentIndex];
        draft.microphone = microphone.currentIndex > 0 ? microphone.model[microphone.currentIndex].value : "disabled";
        draft.systemVolume = systemVolume.value;
        draft.micVolume = micVolume.value;
        draft.closeToTray = closeTray.checked;
        draft.autoCapture = automatic.checked;
        draft.startMinimized = minimized.checked;
        draft.autoUpdate = autoUpdate.checked;
        draft.alwaysReplay = alwaysReplay.checked;
        draft.gameOverlay = gameOverlay.checked;
        draft.saveKey = saveKey.model[saveKey.currentIndex].value;
        draft.markerKey = markerKey.model[markerKey.currentIndex].value;
        draft.micKey = micKey.model[micKey.currentIndex].value;
        draft.pushToTalk = pushToTalk.checked;
        draft.clipSeconds = Number(clipSeconds.text);
        draft.separateTracks = separateTracks.checked;
        draft.saveSound = saveSound.checked;
        draft.cleanupGB = Number(cleanupGB.text);
        draft.cleanupDays = Number(cleanupDays.text);
        appController.apply(draft);
    }
    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Paramètres"
            color: Theme.text
            font.pixelSize: 28
            font.weight: Font.DemiBold
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Actualiser les fenêtres"
            onClicked: appController.refreshWindows()
        }
        ActionButton {
            text: "Appliquer"
            primary: true
            enabled: settings.stopped
            onClicked: settings.apply()
        }
    }
    Text {
        visible: !settings.stopped
        text: "Patiente : la capture est occupée (démarrage ou sauvegarde)."
        color: Theme.accent
    }
    ScrollView {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width
            spacing: 20
            Text {
                text: "Capture"
                color: Theme.text
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            GridLayout {
                columns: 2
                columnSpacing: 24
                rowSpacing: 12
                Layout.fillWidth: true
                enabled: settings.stopped
                Label {
                    text: "Source vidéo"
                    Layout.preferredWidth: 180
                }
                CCombo {
                    id: mode
                    Layout.fillWidth: true
                    model: ["Écran · DXGI", "Fenêtre · WGC", "Jeu · OBS"]
                    currentIndex: Math.max(0, ["display", "window", "game"].indexOf(settings.draft.mode))
                }
                Label {
                    text: "Écran ou fenêtre"
                }
                CCombo {
                    id: target
                    Layout.fillWidth: true
                    textRole: "name"
                    model: settings.rowsForMode()
                    onModelChanged: settings.restoreTarget()
                    onCurrentIndexChanged: if (currentIndex >= 0) settings.chosenTarget = model[currentIndex].value
                    Component.onCompleted: settings.restoreTarget()
                }
                Label {
                    text: ""
                    visible: target.count === 0
                }
                Text {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    visible: target.count === 0
                    text: mode.currentIndex === 0 ? "Aucun écran détecté. Utilise Actualiser après un changement d’écran." : "Aucune fenêtre détectée. Ouvre le jeu ou l’application, puis actualise la liste."
                    color: Theme.secondary
                    wrapMode: Text.Wrap
                    font.pixelSize: 12
                }
                Label {
                    text: "Durée du replay (s)"
                }
                CField {
                    id: seconds
                    objectName: "replaySecondsField"
                    text: settings.draft.replaySeconds
                    validator: IntValidator {
                        bottom: 5
                        top: 3600
                    }
                    Layout.fillWidth: true
                }
                Label {
                    text: "Mémoire du tampon (Mo)"
                }
                CField {
                    id: memory
                    text: settings.draft.memoryMB
                    validator: IntValidator {
                        bottom: 64
                        top: 8192
                    }
                    Layout.fillWidth: true
                }
                Label {
                    text: "Durée d'un clip (s)"
                }
                CField {
                    id: clipSeconds
                    text: settings.draft.clipSeconds
                    placeholderText: "0 = tout le tampon"
                    validator: IntValidator {
                        bottom: 0
                        top: 3600
                    }
                    Layout.fillWidth: true
                }
            }
            Text {
                Layout.fillWidth: true
                text: "La touche de sauvegarde garde les dernières secondes définies par « Durée d'un clip » (0 = tout le tampon) ; Maj + la touche garde toujours tout le tampon. 15 · 30 · 60 · 90 · 120 · 180 · 300 · 600 secondes, ou durée personnalisée. Le moteur conserve des paquets compressés en mémoire. Le tampon temporaire sur disque n'est pas disponible."
                wrapMode: Text.Wrap
                color: Theme.secondary
                font.pixelSize: 12
            }
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: Theme.line
            }
            Text {
                text: "Vidéo"
                color: Theme.text
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            GridLayout {
                columns: 2
                columnSpacing: 24
                rowSpacing: 12
                Layout.fillWidth: true
                enabled: settings.stopped
                Label {
                    text: "Résolution"
                    Layout.preferredWidth: 180
                }
                RowLayout {
                    Layout.fillWidth: true
                    CField {
                        id: widthInput
                        text: settings.draft.width
                        Layout.fillWidth: true
                        validator: IntValidator {
                            bottom: 320
                            top: 7680
                        }
                    }
                    Label {
                        text: "×"
                    }
                    CField {
                        id: heightInput
                        text: settings.draft.height
                        Layout.fillWidth: true
                        validator: IntValidator {
                            bottom: 180
                            top: 4320
                        }
                    }
                    CCombo {
                        model: ["Personnalisée", "720p", "1080p", "1440p", "2160p"]
                        onActivated: {
                            if (currentIndex > 0) {
                                widthInput.text = [0, 1280, 1920, 2560, 3840][currentIndex];
                                heightInput.text = [0, 720, 1080, 1440, 2160][currentIndex];
                            }
                        }
                    }
                }
                Label {
                    text: "Fréquence"
                }
                CCombo {
                    id: fps
                    Layout.fillWidth: true
                    model: ["30 FPS", "60 FPS", "120 FPS", "144 FPS"]
                    currentIndex: [30, 60, 120, 144].indexOf(settings.draft.fps)
                }
                Label {
                    text: "Codec"
                }
                CCombo {
                    id: codec
                    Layout.fillWidth: true
                    textRole: "name"
                    model: [
                        { name: "H.264 · logiciel ou matériel", id: "h264", available: true },
                        { name: "HEVC · matériel requis", id: "hevc", available: appController.encoders.some(function (e) { return e.codec === "hevc" }) },
                        { name: "AV1 · matériel requis", id: "av1", available: appController.encoders.some(function (e) { return e.codec === "av1" }) }
                    ]
                    currentIndex: ["h264", "hevc", "av1"].indexOf(settings.draft.codec)
                    onActivated: {
                        if (model[currentIndex].available !== false) {
                            settings.draft.encoder = "auto"
                            encoder.currentIndex = 0
                        }
                    }
                }
                Label {
                    text: "Encodeur"
                }
                CCombo {
                    id: encoder
                    Layout.fillWidth: true
                    textRole: "name"
                    model: settings.encoderRows()
                    onModelChanged: settings.restoreEncoder()
                    Component.onCompleted: settings.restoreEncoder()
                }
                Label {
                    text: "Débit (kbit/s)"
                }
                CField {
                    id: bitrate
                    Layout.fillWidth: true
                    text: settings.draft.bitrate
                    validator: IntValidator {
                        bottom: 500
                        top: 200000
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: Theme.line
            }
            Text {
                text: "Audio"
                color: Theme.text
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            GridLayout {
                columns: 2
                columnSpacing: 24
                rowSpacing: 12
                Layout.fillWidth: true
                enabled: settings.stopped
                Label {
                    text: "Son à enregistrer"
                    Layout.preferredWidth: 180
                }
                CCombo {
                    id: audio
                    Layout.fillWidth: true
                    textRole: "name"
                    model: [
                        { name: "Aucun", id: "disabled", available: true },
                        { name: "Système entier", id: "system", available: true },
                        { name: "Application sélectionnée seulement", id: "process", available: mode.currentIndex !== 0 && !(mode.currentIndex === 2 && target.currentIndex === 0) }
                    ]
                    currentIndex: ["disabled", "system", "process"].indexOf(settings.draft.audio)
                }
                Label {
                    text: "Volume du son"
                }
                CSlider {
                    id: systemVolume
                    from: 0
                    to: 2
                    value: settings.draft.systemVolume
                    Layout.fillWidth: true
                }
                Label {
                    text: "Microphone"
                }
                CCombo {
                    id: microphone
                    Layout.fillWidth: true
                    textRole: "name"
                    model: [
                        {
                            name: "Désactivé",
                            value: "disabled"
                        },
                        {
                            name: "Micro par défaut de Windows",
                            value: "default"
                        }
                    ].concat(appController.microphones.filter(function (m) { return m.value !== "default" }))
                    Component.onCompleted: {
                        for (var i = 0; i < model.length; i++)
                            if (model[i].value === settings.draft.microphone)
                                currentIndex = i;
                    }
                    onModelChanged: {
                        var found = false
                        for (var i = 0; i < count; ++i)
                            if (model[i].value === settings.draft.microphone) { currentIndex = i; found = true; break }
                        if (!found) currentIndex = 0
                    }
                }
                Label {
                    text: "Volume du micro"
                }
                CSlider {
                    id: micVolume
                    from: 0
                    to: 2
                    value: settings.draft.micVolume
                    Layout.fillWidth: true
                }
            }
            CheckBox {
                id: separateTracks
                enabled: settings.stopped
                text: "Pistes séparées : jeu et micro enregistrés à part (réglables dans l'éditeur)"
                checked: settings.draft.separateTracks
            }
            Text {
                text: (separateTracks.checked ? "Trois pistes AAC : le mix, le jeu seul et le micro seul. Les lecteurs lisent la première." : "Une piste AAC mixée.") + " Le mode application requiert une fenêtre sélectionnée et un Windows compatible ; aucun repli silencieux vers tout le système."
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                color: Theme.secondary
                font.pixelSize: 12
            }
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: Theme.line
            }
            Text {
                text: "Raccourcis globaux"
                color: Theme.text
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            GridLayout {
                columns: 2
                Layout.fillWidth: true
                columnSpacing: 24
                rowSpacing: 12
                Label {
                    text: "Sauvegarder"
                    Layout.preferredWidth: 180
                }
                CCombo {
                    id: saveKey
                    textRole: "name"
                    model: settings.keyRows(settings.draft.saveKey)
                    currentIndex: settings.draft.saveKey - 112
                    Layout.fillWidth: true
                }
                Label {
                    text: "Poser un marqueur"
                }
                CCombo {
                    id: markerKey
                    textRole: "name"
                    model: settings.optionalKeyRows()
                    currentIndex: settings.keyIndex(model, settings.draft.markerKey)
                    Layout.fillWidth: true
                }
                Label {
                    text: "Micro (couper / parler)"
                }
                CCombo {
                    id: micKey
                    textRole: "name"
                    model: settings.optionalKeyRows()
                    currentIndex: settings.keyIndex(model, settings.draft.micKey)
                    Layout.fillWidth: true
                }
            }
            CheckBox {
                id: pushToTalk
                enabled: micKey.currentIndex > 0
                text: "Push-to-talk : le micro n'est enregistré que tant que la touche est maintenue"
                checked: settings.draft.pushToTalk
            }
            CheckBox {
                id: saveSound
                text: "Jouer un son quand un clip est sauvegardé"
                checked: settings.draft.saveSound
            }
            Text {
                Layout.fillWidth: true
                text: "Le marqueur repère un moment dans le tampon ; il apparaît sur la timeline de l'éditeur (N / Maj+N pour sauter d'un marqueur à l'autre)."
                wrapMode: Text.Wrap
                color: Theme.secondary
                font.pixelSize: 12
            }
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: Theme.line
            }
            Text {
                text: "Général et stockage"
                color: Theme.text
                font.pixelSize: 19
                font.weight: Font.DemiBold
            }
            CheckBox {
                id: alwaysReplay
                text: "Instant Replay toujours actif dès le lancement de ClipForge"
                checked: settings.draft.alwaysReplay
            }
            CheckBox {
                id: closeTray
                text: "Fermer la fenêtre masque l'application dans la zone de notification"
                checked: settings.draft.closeToTray
            }
            CheckBox {
                id: automatic
                text: "Activer automatiquement le replay pour mes profils de jeu"
                checked: settings.draft.autoCapture
            }
            CheckBox {
                id: minimized
                text: "Démarrer avec la fenêtre masquée"
                checked: settings.draft.startMinimized
            }
            CheckBox {
                text: "Lancer ClipForge au démarrage de Windows (dans la zone de notification)"
                checked: appController.autostart
                onToggled: appController.setAutostart(checked)
            }
            CheckBox {
                id: gameOverlay
                text: "Afficher une notification en jeu (3 s) quand un clip est sauvegardé"
                checked: settings.draft.gameOverlay
            }
            CheckBox {
                id: autoUpdate
                text: "Télécharger automatiquement les mises à jour"
                checked: settings.draft.autoUpdate
            }
            RowLayout {
                spacing: 10
                ActionButton {
                    text: appController.updateState === "available" ? "Télécharger " + appController.updateVersion
                        : appController.updateState === "ready" ? "Redémarrer et installer " + appController.updateVersion
                        : appController.updateState === "downloading" ? "Téléchargement " + Math.round(appController.updateProgress * 100) + " %"
                        : "Rechercher une mise à jour"
                    enabled: appController.updateState !== "checking" && appController.updateState !== "downloading"
                    onClicked: appController.updateState === "idle" ? appController.checkForUpdates() : appController.installUpdate()
                }
            }
            Text {
                Layout.fillWidth: true
                text: appController.config.clips
                color: Theme.secondary
                elide: Text.ElideMiddle
            }
            RowLayout {
                spacing: 10
                ActionButton {
                    text: "Changer de dossier"
                    enabled: settings.stopped
                    onClicked: appController.chooseClipsFolder()
                }
                ActionButton {
                    text: "Ouvrir le dossier"
                    onClicked: appController.reveal("")
                }
            }
            GridLayout {
                columns: 2
                columnSpacing: 24
                rowSpacing: 12
                Layout.fillWidth: true
                Label {
                    text: "Espace max des clips (Go)"
                    Layout.preferredWidth: 180
                }
                CField {
                    id: cleanupGB
                    text: settings.draft.cleanupGB
                    placeholderText: "0 = illimité"
                    validator: IntValidator {
                        bottom: 0
                        top: 100000
                    }
                    Layout.fillWidth: true
                }
                Label {
                    text: "Garder les clips (jours)"
                }
                CField {
                    id: cleanupDays
                    text: settings.draft.cleanupDays
                    placeholderText: "0 = pour toujours"
                    validator: IntValidator {
                        bottom: 0
                        top: 3650
                    }
                    Layout.fillWidth: true
                }
            }
            RowLayout {
                spacing: 10
                ActionButton {
                    text: "Nettoyer maintenant"
                    enabled: Number(appController.config.cleanupGB) > 0 || Number(appController.config.cleanupDays) > 0
                    onClicked: appController.cleanupNow()
                }
            }
            Text {
                Layout.fillWidth: true
                text: Number(cleanupGB.text) > 0 || Number(cleanupDays.text) > 0 ? "Au-delà de ces limites, les clips les plus anciens partent dans la corbeille Windows (restaurables). Les favoris ne sont jamais supprimés. Une réserve de 256 Mo est requise pour démarrer une capture." : "Aucun clip sauvegardé n'est supprimé automatiquement (0 = sans limite). Une réserve de 256 Mo est requise pour démarrer une capture."
                color: Theme.secondary
                wrapMode: Text.Wrap
                font.pixelSize: 12
            }
            RowLayout {
                spacing: 10
                ActionButton {
                    text: "Importer les réglages"
                    enabled: settings.stopped
                    onClicked: appController.configurationFile(true)
                }
                ActionButton {
                    text: "Exporter les réglages"
                    onClicked: appController.configurationFile(false)
                }
            }
            Text {
                Layout.topMargin: 16
                text: "ClipForge " + appController.version + " · C++20 / Qt 6 / libobs / FFmpeg / SQLite\nLogiciel libre GPLv3 · local, sans compte, sans télémétrie"
                color: Theme.secondary
                font.pixelSize: 12
                lineHeight: 1.7
            }
            Item {
                height: 14
            }
        }
    }
}
