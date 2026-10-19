import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import ClipForge.Playback 1.0
import "../theme"
import "../components"

ColumnLayout {
    id: editor
    spacing: 14
    readonly property bool hasClip: !!appController.selected.path
    readonly property var speeds: [0.25, 0.5, 1, 2, 4]
    readonly property real clipLength: Math.max(0, Number(appController.selected.duration || player.duration / 1000))
    property real inPoint: 0
    property real outPoint: Number(appController.selected.duration || 0)
    // Set while "Lire le passage" plays the selected range; playback pauses at the out point.
    property bool previewing: false
    readonly property real outputSeconds: {
        var total = 0;
        for (var i = 0; i < appController.segments.length; ++i) {
            var s = appController.segments[i];
            total += (s.out - s.in) / s.speed;
        }
        return total;
    }
    onInPointChanged: if (trimRange)
        trimRange.first.value = inPoint
    onOutPointChanged: if (trimRange)
        trimRange.second.value = outPoint
    function seekTo(seconds) {
        var ms = Math.round(Math.max(0, Math.min(player.duration, seconds * 1000)));
        playback.seek(ms);
    }
    function togglePlay() {
        if (!hasClip)
            return;
        previewing = false;
        player.playbackState === MediaPlayer.PlayingState ? player.pause() : player.play();
    }
    function previewRange() {
        if (!hasClip || outPoint <= inPoint)
            return;
        seekTo(inPoint);
        previewing = true;
        player.play();
    }
    function setIn() {
        if (!hasClip)
            return;
        inPoint = Math.min(player.position / 1000, Math.max(0, outPoint - 0.1));
    }
    function setOut() {
        if (!hasClip)
            return;
        outPoint = Math.max(player.position / 1000, inPoint + 0.1);
    }

    Shortcut {
        sequence: "Space"
        enabled: editor.hasClip
        onActivated: editor.togglePlay()
    }
    Shortcut {
        sequence: "I"
        enabled: editor.hasClip
        onActivated: editor.setIn()
    }
    Shortcut {
        sequence: "O"
        enabled: editor.hasClip
        onActivated: editor.setOut()
    }
    Shortcut {
        sequence: "P"
        enabled: editor.hasClip
        onActivated: editor.previewRange()
    }
    Shortcut {
        sequence: "Left"
        enabled: editor.hasClip
        onActivated: editor.seekTo(player.position / 1000 - 1)
    }
    Shortcut {
        sequence: "Right"
        enabled: editor.hasClip
        onActivated: editor.seekTo(player.position / 1000 + 1)
    }
    Shortcut {
        sequence: "Shift+Left"
        enabled: editor.hasClip
        onActivated: editor.seekTo(player.position / 1000 - 5)
    }
    Shortcut {
        sequence: "Shift+Right"
        enabled: editor.hasClip
        onActivated: editor.seekTo(player.position / 1000 + 5)
    }
    Shortcut {
        sequence: "Home"
        enabled: editor.hasClip
        onActivated: editor.seekTo(editor.inPoint)
    }
    Shortcut {
        sequence: "End"
        enabled: editor.hasClip
        onActivated: editor.seekTo(editor.outPoint)
    }
    Shortcut {
        sequences: [StandardKey.Undo]
        enabled: appController.canUndo
        onActivated: appController.undo()
    }
    Shortcut {
        sequences: [StandardKey.Redo, "Ctrl+Y"]
        enabled: appController.canRedo
        onActivated: appController.redo()
    }

    RowLayout {
        Layout.fillWidth: true
        Text {
            text: "Éditeur"
            color: Theme.text
            font.pixelSize: 28
            font.weight: Font.DemiBold
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Ouvrir un projet"
            onClicked: appController.loadProject()
        }
        ActionButton {
            text: "Enregistrer le projet"
            enabled: editor.hasClip
            onClicked: appController.saveProject()
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 12
        Text {
            text: appController.selected.name || "Choisis un clip dans la bibliothèque pour commencer."
            color: Theme.secondary
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        Text {
            visible: editor.hasClip
            text: "Espace lire · I / O entrée / sortie · P aperçu · ← → 1 s (Maj 5 s)"
            color: Theme.secondary
            opacity: 0.8
            font.pixelSize: 11
        }
    }
    Text {
        visible: playback.error.length > 0
        text: playback.error
        color: Theme.danger
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        font.pixelSize: 12
    }
    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 150
        color: "#07080d"
        radius: 8
        VideoOutput {
            id: output
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectFit
        }
        MediaPlayer {
            id: player
            objectName: "clipPlayer"
            source: window.fileUrl(appController.selected.path)
            videoOutput: output
            onDurationChanged: function (duration) {
                if (duration > 0)
                    editor.outPoint = duration / 1000;
            }
            onPositionChanged: function (position) {
                if (editor.previewing && position >= editor.outPoint * 1000) {
                    editor.previewing = false;
                    player.pause();
                }
            }
            onPlaybackStateChanged: if (playbackState !== MediaPlayer.PlayingState)
                editor.previewing = false
        }
        PlaybackAudio {
            id: playback
            objectName: "playbackAudio"
            mediaPlayer: player
            volume: volumeControl.value
        }
        TapHandler {
            enabled: editor.hasClip
            onTapped: editor.togglePlay()
        }
        Text {
            visible: !editor.hasClip
            anchors.centerIn: parent
            text: "Aucune vidéo ouverte"
            color: Theme.secondary
        }
        Text {
            visible: player.error !== MediaPlayer.NoError
            anchors.centerIn: parent
            width: parent.width - 40
            text: "Lecture impossible : " + player.errorString
            color: Theme.danger
            wrapMode: Text.Wrap
        }
    }
    RowLayout {
        Layout.fillWidth: true
        ActionButton {
            text: player.playbackState === MediaPlayer.PlayingState ? "Pause" : "Lire"
            enabled: editor.hasClip
            onClicked: editor.togglePlay()
        }
        Text {
            text: Theme.duration(player.position / 1000, true) + " / " + Theme.duration(player.duration / 1000, true)
            color: Theme.text
            font.family: Theme.mono
            Layout.preferredWidth: 150
            horizontalAlignment: Text.AlignHCenter
        }
        CSlider {
            id: seekBar
            Layout.fillWidth: true
            from: 0
            to: Math.max(1, player.duration)
            value: player.position
            onMoved: playback.seek(value)
        }
        CCombo {
            id: speed
            implicitWidth: 96
            model: ["0.25×", "0.5×", "1×", "2×", "4×"]
            currentIndex: 2
            onActivated: player.playbackRate = editor.speeds[currentIndex]
        }
    }
    RangeSlider {
        id: trimRange
        Layout.fillWidth: true
        implicitHeight: 32
        enabled: editor.hasClip
        from: 0
        to: Math.max(1, editor.clipLength)
        onToChanged: Qt.callLater(function () {
            trimRange.first.value = editor.inPoint;
            trimRange.second.value = editor.outPoint;
        })
        first.value: editor.inPoint
        second.value: editor.outPoint
        first.onMoved: editor.inPoint = first.value
        second.onMoved: editor.outPoint = second.value
        background: Rectangle {
            x: trimRange.leftPadding
            y: trimRange.topPadding + trimRange.availableHeight / 2 - height / 2
            width: trimRange.availableWidth
            height: 8
            radius: 4
            color: Theme.line
            Rectangle {
                x: trimRange.first.visualPosition * parent.width
                width: (trimRange.second.visualPosition - trimRange.first.visualPosition) * parent.width
                height: parent.height
                radius: 4
                color: Theme.accentSurface
                border.color: Theme.accent
            }
            // Playhead within the trim track.
            Rectangle {
                x: (player.duration > 0 ? player.position / player.duration : 0) * parent.width - 1
                y: -4
                width: 2
                height: parent.height + 8
                color: Theme.text
                opacity: 0.7
            }
        }
        first.handle: Rectangle {
            x: trimRange.leftPadding + trimRange.first.visualPosition * (trimRange.availableWidth - width)
            y: trimRange.topPadding + (trimRange.availableHeight - height) / 2
            width: 10
            height: 24
            radius: 3
            color: Theme.accent
            border.width: trimRange.first.handle.activeFocus || trimRange.first.pressed ? 2 : 0
            border.color: Theme.text
        }
        second.handle: Rectangle {
            x: trimRange.leftPadding + trimRange.second.visualPosition * (trimRange.availableWidth - width)
            y: trimRange.topPadding + (trimRange.availableHeight - height) / 2
            width: 10
            height: 24
            radius: 3
            color: Theme.accent
            border.width: trimRange.second.handle.activeFocus || trimRange.second.pressed ? 2 : 0
            border.color: Theme.text
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 8
        ActionButton {
            text: "Entrée ici"
            onClicked: editor.setIn()
            enabled: editor.hasClip
        }
        Text {
            text: Theme.duration(editor.inPoint, true)
            color: Theme.secondary
            font.family: Theme.mono
        }
        ActionButton {
            text: "Sortie ici"
            onClicked: editor.setOut()
            enabled: editor.hasClip
        }
        Text {
            text: Theme.duration(editor.outPoint, true)
            color: Theme.secondary
            font.family: Theme.mono
        }
        Text {
            text: "· " + Theme.duration(Math.max(0, editor.outPoint - editor.inPoint), true)
            color: Theme.accent
            font.family: Theme.mono
        }
        ActionButton {
            text: editor.previewing ? "Aperçu…" : "Lire le passage"
            enabled: editor.hasClip && editor.outPoint > editor.inPoint
            onClicked: editor.previewRange()
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Ajouter"
            enabled: editor.outPoint > editor.inPoint && editor.hasClip
            onClicked: appController.addSegment(editor.inPoint, editor.outPoint, editor.speeds[speed.currentIndex])
        }
        ActionButton {
            text: "Garder ce passage"
            enabled: editor.outPoint > editor.inPoint && editor.hasClip
            primary: true
            onClicked: appController.useSegment(editor.inPoint, editor.outPoint, editor.speeds[speed.currentIndex])
        }
    }
    RowLayout {
        Text {
            text: "Passages exportés dans cet ordre"
            color: Theme.secondary
            font.pixelSize: 12
        }
        Text {
            visible: appController.segments.length > 0
            text: "· durée finale " + Theme.duration(editor.outputSeconds, true)
            color: Theme.text
            font.family: Theme.mono
            font.pixelSize: 12
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Annuler"
            enabled: appController.canUndo
            onClicked: appController.undo()
        }
        ActionButton {
            text: "Rétablir"
            enabled: appController.canRedo
            onClicked: appController.redo()
        }
    }
    ListView {
        Layout.fillWidth: true
        Layout.preferredHeight: 88
        clip: true
        model: appController.segments
        spacing: 4
        ScrollBar.vertical: ScrollBar {}
        delegate: Rectangle {
            required property var modelData
            required property int index
            width: ListView.view.width
            height: 40
            color: segmentHover.hovered ? Theme.raised : Theme.surface
            radius: 6
            HoverHandler {
                id: segmentHover
            }
            TapHandler {
                // Clicking a passage loads its range back into the trim controls.
                onTapped: {
                    editor.inPoint = modelData.in;
                    editor.outPoint = modelData.out;
                    editor.seekTo(modelData.in);
                }
            }
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 6
                Text {
                    Layout.fillWidth: true
                    text: (index + 1) + "   " + Theme.duration(modelData.in, true) + " → " + Theme.duration(modelData.out, true) + "   ·   " + modelData.speed + "×"
                    color: Theme.text
                    font.family: Theme.mono
                }
                ActionButton {
                    text: "Retirer"
                    implicitHeight: 30
                    onClicked: appController.removeSegment(index)
                }
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        Text {
            text: "Volume " + Math.round(volumeControl.value * 100) + " %"
            color: Theme.secondary
            Layout.preferredWidth: 96
        }
        CSlider {
            id: volumeControl
            from: 0
            to: 2
            value: 1
            stepSize: 0.05
            Layout.preferredWidth: 100
        }
        CCombo {
            id: resolution
            model: ["Source", "720p", "1080p", "1440p", "2160p"]
        }
        CCombo {
            id: fps
            model: ["30 FPS", "60 FPS", "120 FPS"]
            currentIndex: 1
        }
        CCombo {
            id: codec
            model: ["H.264", "HEVC"]
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            visible: !appController.exportBusy && appController.lastExport.length > 0
            text: "Afficher l'export"
            onClicked: appController.reveal(appController.lastExport)
        }
        ActionButton {
            text: appController.exportBusy ? "Export " + Math.round(appController.exportProgress * 100) + " %" : "Exporter en MP4"
            primary: true
            enabled: !appController.exportBusy && appController.segments.length > 0
            onClicked: appController.exportClip([0, 720, 1080, 1440, 2160][resolution.currentIndex], [30, 60, 120][fps.currentIndex], codec.currentIndex === 0 ? "h264" : "hevc", volumeControl.value)
        }
    }
    Rectangle {
        visible: appController.exportBusy
        Layout.fillWidth: true
        Layout.preferredHeight: 4
        radius: 2
        color: Theme.line
        Rectangle {
            width: parent.width * appController.exportProgress
            height: parent.height
            radius: 2
            color: Theme.accent
            Behavior on width {
                NumberAnimation {
                    duration: 150
                }
            }
        }
    }
    Text {
        Layout.fillWidth: true
        text: "Montage non destructif · export précis avec réencodage · la vidéo originale reste intacte."
        color: Theme.secondary
        font.pixelSize: 11
    }
    Component.onDestruction: player.stop()
}
