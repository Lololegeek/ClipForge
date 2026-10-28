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
    property real volumeBeforeMute: 1
    property bool cropping: false
    readonly property var markers: appController.selected.markers || []
    readonly property bool separateAudio: (appController.edit.audioTracks || 1) >= 3
    // Export presets: index 0 uses the manual resolution / FPS / codec choices.
    readonly property var presets: [
        { name: "Personnalisé" },
        { name: "Discord ≤ 10 Mo", format: "mp4", maxMB: 10, fps: 60 },
        { name: "YouTube 1080p60", format: "mp4", height: 1080, fps: 60, crf: 18 },
        { name: "Vertical 9:16", format: "mp4", vertical: true, height: 1920, fps: 60 },
        { name: "GIF animé", format: "gif", height: 360, fps: 15 },
        { name: "WebP animé", format: "webp", height: 480, fps: 20 }
    ]
    function exportOptions() {
        var o = {
            volume: volumeControl.value,
            gameVolume: gameVolume.value,
            micVolume: micVolume.value
        };
        if (preset.currentIndex === 0) {
            o.format = "mp4";
            o.height = [0, 720, 1080, 1440, 2160][resolution.currentIndex];
            o.fps = [30, 60, 120][fps.currentIndex];
            o.codec = codec.currentIndex === 0 ? "h264" : "hevc";
            return o;
        }
        var p = presets[preset.currentIndex];
        for (var k in p)
            if (k !== "name")
                o[k] = p[k];
        return o;
    }
    function nextMarker(forward) {
        var now = player.position / 1000, best = -1;
        for (var i = 0; i < markers.length; ++i) {
            var m = markers[i];
            if (forward ? (m > now + 0.2 && (best < 0 || m < best)) : (m < now - 0.2 && (best < 0 || m > best)))
                best = m;
        }
        if (best >= 0)
            seekTo(best);
    }
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
        sequence: ","
        enabled: editor.hasClip
        onActivated: {
            player.pause();
            editor.seekTo(player.position / 1000 - 1 / 30);
        }
    }
    Shortcut {
        sequence: "."
        enabled: editor.hasClip
        onActivated: {
            player.pause();
            editor.seekTo(player.position / 1000 + 1 / 30);
        }
    }
    Shortcut {
        sequence: "M"
        enabled: editor.hasClip
        onActivated: {
            if (volumeControl.value > 0) {
                editor.volumeBeforeMute = volumeControl.value;
                volumeControl.value = 0;
            } else {
                volumeControl.value = editor.volumeBeforeMute;
            }
        }
    }
    Shortcut {
        sequence: "N"
        enabled: editor.hasClip && editor.markers.length > 0
        onActivated: editor.nextMarker(true)
    }
    Shortcut {
        sequence: "Shift+N"
        enabled: editor.hasClip && editor.markers.length > 0
        onActivated: editor.nextMarker(false)
    }
    Shortcut {
        sequence: "Escape"
        enabled: editor.cropping
        onActivated: editor.cropping = false
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
            text: "Espace lire · I / O entrée / sortie · P aperçu · ← → 1 s (Maj 5 s) · , . image · M muet · N marqueur"
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
        // The library thumbnail shows instantly while the decoder opens the file.
        Image {
            anchors.fill: parent
            visible: editor.hasClip && player.playbackState === MediaPlayer.StoppedState && player.position === 0
            source: appController.selected.thumbnail ? window.fileUrl(appController.selected.thumbnail) : ""
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            cache: true
        }
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
            enabled: editor.hasClip && !editor.cropping
            onTapped: editor.togglePlay()
        }
        Item {
            id: cropLayer
            // Normalised crop being edited; committed to the project when a drag ends.
            property real cx: 0
            property real cy: 0
            property real cw: 1
            property real ch: 1
            property real startX: 0
            property real startY: 0
            property real startW: 1
            property real startH: 1
            visible: editor.hasClip && (editor.cropping || !!appController.edit.cropped)
            x: output.contentRect.x
            y: output.contentRect.y
            width: output.contentRect.width
            height: output.contentRect.height
            function sync() {
                var c = appController.edit.crop || [0, 0, 1, 1];
                cx = c[0];
                cy = c[1];
                cw = c[2];
                ch = c[3];
            }
            function commit() {
                appController.setCrop(cx, cy, cw, ch);
            }
            // Width / height of the crop in pixels for the chosen aspect, 0 when free.
            function ratio() {
                return [0, 16 / 9, 9 / 16, 1][aspect.currentIndex];
            }
            function applyRatio() {
                var r = ratio();
                if (r <= 0 || width <= 0 || height <= 0)
                    return;
                ch = Math.min(1, cw * width / r / height);
                cw = Math.min(cw, ch * height * r / width);
                cx = Math.max(0, Math.min(cx, 1 - cw));
                cy = Math.max(0, Math.min(cy, 1 - ch));
                commit();
            }
            Component.onCompleted: sync()
            Connections {
                target: appController
                function onEditChanged() {
                    if (!moveDrag.active && !sizeDrag.active)
                        cropLayer.sync();
                }
            }
            // Dimmed area outside the crop.
            Rectangle {
                width: parent.width
                height: cropBox.y
                color: "#a0000000"
            }
            Rectangle {
                y: cropBox.y + cropBox.height
                width: parent.width
                height: parent.height - y
                color: "#a0000000"
            }
            Rectangle {
                y: cropBox.y
                width: cropBox.x
                height: cropBox.height
                color: "#a0000000"
            }
            Rectangle {
                x: cropBox.x + cropBox.width
                y: cropBox.y
                width: parent.width - x
                height: cropBox.height
                color: "#a0000000"
            }
            Rectangle {
                id: cropBox
                x: cropLayer.cx * cropLayer.width
                y: cropLayer.cy * cropLayer.height
                width: cropLayer.cw * cropLayer.width
                height: cropLayer.ch * cropLayer.height
                color: "transparent"
                border.color: Theme.accent
                border.width: editor.cropping ? 2 : 1
                DragHandler {
                    id: moveDrag
                    enabled: editor.cropping
                    target: null
                    cursorShape: Qt.SizeAllCursor
                    onActiveChanged: {
                        if (active) {
                            cropLayer.startX = cropLayer.cx;
                            cropLayer.startY = cropLayer.cy;
                        } else {
                            cropLayer.commit();
                        }
                    }
                    onTranslationChanged: if (active) {
                        cropLayer.cx = Math.max(0, Math.min(1 - cropLayer.cw, cropLayer.startX + translation.x / cropLayer.width));
                        cropLayer.cy = Math.max(0, Math.min(1 - cropLayer.ch, cropLayer.startY + translation.y / cropLayer.height));
                    }
                }
                Rectangle {
                    visible: editor.cropping
                    width: 16
                    height: 16
                    radius: 3
                    x: parent.width - width / 2
                    y: parent.height - height / 2
                    color: Theme.accent
                    border.color: Theme.text
                    DragHandler {
                        id: sizeDrag
                        target: null
                        cursorShape: Qt.SizeFDiagCursor
                        onActiveChanged: {
                            if (active) {
                                cropLayer.startW = cropLayer.cw;
                                cropLayer.startH = cropLayer.ch;
                            } else {
                                cropLayer.commit();
                            }
                        }
                        onTranslationChanged: if (active) {
                            var w = Math.max(0.05, Math.min(1 - cropLayer.cx, cropLayer.startW + translation.x / cropLayer.width));
                            var h = Math.max(0.05, Math.min(1 - cropLayer.cy, cropLayer.startH + translation.y / cropLayer.height));
                            var r = cropLayer.ratio();
                            if (r > 0) {
                                h = w * cropLayer.width / r / cropLayer.height;
                                if (cropLayer.cy + h > 1) {
                                    h = 1 - cropLayer.cy;
                                    w = h * cropLayer.height * r / cropLayer.width;
                                }
                            }
                            cropLayer.cw = w;
                            cropLayer.ch = h;
                        }
                    }
                }
            }
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
        ActionButton {
            text: "Capturer l'image"
            enabled: editor.hasClip
            onClicked: appController.captureFrame(player.position / 1000)
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 8
        visible: editor.hasClip
        ActionButton {
            text: editor.cropping ? "Terminer le recadrage" : appController.edit.cropped ? "Modifier le recadrage" : "Recadrer"
            primary: editor.cropping
            onClicked: {
                if (!editor.cropping)
                    cropLayer.sync();
                editor.cropping = !editor.cropping;
            }
        }
        CCombo {
            id: aspect
            visible: editor.cropping
            implicitWidth: 110
            model: ["Libre", "16:9", "9:16", "1:1"]
            onActivated: cropLayer.applyRatio()
        }
        ActionButton {
            visible: !!appController.edit.cropped
            text: "Retirer le recadrage"
            onClicked: appController.setCrop(0, 0, 1, 1)
        }
        Text {
            visible: editor.cropping
            text: "Glisse le cadre pour le déplacer, la poignée pour le redimensionner."
            color: Theme.secondary
            font.pixelSize: 12
        }
        Item {
            Layout.fillWidth: true
        }
        Text {
            visible: editor.markers.length > 0
            text: editor.markers.length + (editor.markers.length > 1 ? " marqueurs" : " marqueur") + " · N / Maj+N pour y aller"
            color: Theme.secondary
            font.pixelSize: 12
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
            Repeater {
                model: editor.markers
                Rectangle {
                    required property var modelData
                    x: modelData / Math.max(1, editor.clipLength) * parent.width - 2
                    y: -7
                    width: 4
                    height: parent.height + 14
                    radius: 2
                    color: Theme.success
                }
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
        // Clips recorded with separate tracks rebuild their mix from the game and microphone levels.
        Text {
            visible: editor.separateAudio
            text: "Jeu " + Math.round(gameVolume.value * 100) + " %"
            color: Theme.secondary
            Layout.preferredWidth: 80
        }
        CSlider {
            id: gameVolume
            visible: editor.separateAudio
            from: 0
            to: 2
            value: 1
            stepSize: 0.05
            Layout.preferredWidth: 100
        }
        Text {
            visible: editor.separateAudio
            text: "Micro " + Math.round(micVolume.value * 100) + " %"
            color: Theme.secondary
            Layout.preferredWidth: 80
        }
        CSlider {
            id: micVolume
            visible: editor.separateAudio
            from: 0
            to: 2
            value: 1
            stepSize: 0.05
            Layout.preferredWidth: 100
        }
        Item {
            Layout.fillWidth: true
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 10
        CCombo {
            id: preset
            implicitWidth: 180
            model: editor.presets.map(function (p) {
                return p.name;
            })
        }
        CCombo {
            id: resolution
            visible: preset.currentIndex === 0
            model: ["Source", "720p", "1080p", "1440p", "2160p"]
        }
        CCombo {
            id: fps
            visible: preset.currentIndex === 0
            model: ["30 FPS", "60 FPS", "120 FPS"]
            currentIndex: 1
        }
        CCombo {
            id: codec
            visible: preset.currentIndex === 0
            model: ["H.264", "HEVC"]
        }
        Text {
            visible: preset.currentIndex > 0
            text: ["", "MP4 H.264, débit calculé pour rester sous 10 Mo", "MP4 1080p, 60 FPS, haute qualité", "Recadré au centre en 1080×1920", "15 FPS, 360p, sans son", "20 FPS, 480p, sans son"][preset.currentIndex]
            color: Theme.secondary
            font.pixelSize: 12
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
            text: appController.exportBusy ? "Export " + Math.round(appController.exportProgress * 100) + " %" : "Exporter en " + ({ gif: "GIF", webp: "WebP" }[editor.presets[preset.currentIndex].format] || "MP4")
            primary: true
            enabled: !appController.exportBusy && appController.segments.length > 0
            onClicked: appController.exportClip(editor.exportOptions())
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
