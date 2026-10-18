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
    property real inPoint: 0
    property real outPoint: Number(appController.selected.duration || 0)
    onInPointChanged: if (trimRange)
        trimRange.first.value = inPoint
    onOutPointChanged: if (trimRange)
        trimRange.second.value = outPoint
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
            enabled: !!appController.selected.path
            onClicked: appController.saveProject()
        }
    }
    Text {
        text: appController.selected.name || "Choisis un clip dans la bibliothèque pour commencer."
        color: Theme.secondary
        elide: Text.ElideRight
        Layout.fillWidth: true
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
        }
        PlaybackAudio {
            id: playback
            objectName: "playbackAudio"
            mediaPlayer: player
            volume: volumeControl.value
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
            enabled: !!appController.selected.path
            onClicked: player.playbackState === MediaPlayer.PlayingState ? player.pause() : player.play()
        }
        Text {
            text: (player.position / 1000).toFixed(1) + " / " + (player.duration / 1000).toFixed(1) + " s"
            color: Theme.text
            font.family: Theme.mono
            Layout.preferredWidth: 140
        }
        CSlider {
            Layout.fillWidth: true
            from: 0
            to: Math.max(1, player.duration)
            value: player.position
            onMoved: playback.seek(value)
        }
        CCombo {
            id: speed
            model: ["0.25×", "0.5×", "1×", "2×", "4×"]
            currentIndex: 2
            onActivated: player.playbackRate = [0.25, 0.5, 1, 2, 4][currentIndex]
        }
    }
    RangeSlider {
        id: trimRange
        Layout.fillWidth: true
        from: 0
        to: Math.max(1, Number(appController.selected.duration || player.duration / 1000))
        onToChanged: Qt.callLater(function () {
            trimRange.first.value = editor.inPoint;
            trimRange.second.value = editor.outPoint;
        })
        first.value: editor.inPoint
        second.value: editor.outPoint
        first.onMoved: editor.inPoint = first.value
        second.onMoved: editor.outPoint = second.value
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 8
        ActionButton {
            text: "Entrée ici"
            onClicked: editor.inPoint = player.position / 1000
            enabled: !!appController.selected.path
        }
        Text {
            text: editor.inPoint.toFixed(2) + " s"
            color: Theme.secondary
            font.family: Theme.mono
        }
        ActionButton {
            text: "Sortie ici"
            onClicked: editor.outPoint = player.position / 1000
            enabled: !!appController.selected.path
        }
        Text {
            text: editor.outPoint.toFixed(2) + " s"
            color: Theme.secondary
            font.family: Theme.mono
        }
        Item {
            Layout.fillWidth: true
        }
        ActionButton {
            text: "Ajouter"
            enabled: editor.outPoint > editor.inPoint && !!appController.selected.path
            onClicked: appController.addSegment(editor.inPoint, editor.outPoint, [0.25, 0.5, 1, 2, 4][speed.currentIndex])
        }
        ActionButton {
            text: "Garder ce passage"
            enabled: editor.outPoint > editor.inPoint && !!appController.selected.path
            primary: true
            onClicked: appController.useSegment(editor.inPoint, editor.outPoint, [0.25, 0.5, 1, 2, 4][speed.currentIndex])
        }
    }
    RowLayout {
        Text {
            text: "Passages exportés dans cet ordre"
            color: Theme.secondary
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
            color: Theme.surface
            radius: 6
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 6
                Text {
                    Layout.fillWidth: true
                    text: (index + 1) + "   " + modelData.in.toFixed(2) + " → " + modelData.out.toFixed(2) + " s   ·   " + modelData.speed + "×"
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
            text: "Volume"
            color: Theme.secondary
        }
        CSlider {
            id: volumeControl
            from: 0
            to: 2
            value: 1
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
            text: appController.exportBusy ? "Export " + Math.round(appController.exportProgress * 100) + " %" : "Exporter en MP4"
            primary: true
            enabled: !appController.exportBusy && appController.segments.length > 0
            onClicked: appController.exportClip([0, 720, 1080, 1440, 2160][resolution.currentIndex], [30, 60, 120][fps.currentIndex], codec.currentIndex === 0 ? "h264" : "hevc", volumeControl.value)
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
