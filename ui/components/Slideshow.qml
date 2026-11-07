import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import ClipForge.Playback 1.0
import "../theme"

// Plays a list of clips one after another, then shows a recap of the collection, slide by slide.
FocusScope {
    id: show
    visible: false
    property var clips: []
    property int index: 0
    property bool finished: false
    property bool paused: false
    property bool controlsShown: true
    property var stats: ({})
    property var slides: []
    property int statPage: 0
    property int visibilityBefore: Window.Windowed
    readonly property bool fullscreen: Window.window !== null && Window.window.visibility === Window.FullScreen
    readonly property var clip: index >= 0 && index < clips.length ? clips[index] : ({})
    readonly property var dayNames: ["dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"]
    readonly property var monthNames: ["janvier", "février", "mars", "avril", "mai", "juin", "juillet", "août", "septembre", "octobre", "novembre", "décembre"]
    readonly property var palettes: [["#5b4bff", "#ff4b8b"], ["#ff7a18", "#d6246e"], ["#00a383", "#0a6fd6"], ["#6c4ce0", "#00b7c2"], ["#e2367f", "#ff9a5a"], ["#1e8a5a", "#b3c020"]]

    function start(list) {
        if (!list.length)
            return;
        clips = list;
        stats = compute(list);
        slides = buildSlides(stats);
        index = 0;
        finished = false;
        visible = true;
        forceActiveFocus();
        poke();
        load();
    }
    function fileUrl(p) {
        return p ? "file:///" + String(p).replace(/\\/g, "/") : "";
    }
    function load() {
        paused = false;
        player.stop();
        player.source = fileUrl(clip.path);
        player.play();
        intro.restart();
    }
    function next() {
        if (finished)
            return nextStat();
        if (index + 1 < clips.length) {
            index++;
            load();
        } else {
            end();
        }
    }
    function previous() {
        if (finished) {
            statPage = Math.max(0, statPage - 1);
            statTimer.restart();
            return;
        }
        if (player.position > 3000 || index === 0) {
            playback.seek(0);
        } else {
            index--;
            load();
        }
    }
    function togglePause() {
        if (finished)
            return;
        paused = !paused;
        if (paused)
            player.pause();
        else
            player.play();
        poke();
    }
    function end() {
        player.stop();
        player.source = "";
        finished = true;
        statPage = 0;
        statTimer.restart();
        poke();
    }
    function nextStat() {
        if (statPage < slides.length)
            statPage++;
        if (statPage < slides.length)
            statTimer.restart();
        else
            statTimer.stop();
    }
    function replay() {
        statTimer.stop();
        finished = false;
        index = 0;
        load();
    }
    function close() {
        statTimer.stop();
        player.stop();
        player.source = "";
        if (fullscreen)
            setFullscreen(false);
        visible = false;
    }
    function setFullscreen(on) {
        var w = Window.window;
        if (!w)
            return;
        if (on) {
            visibilityBefore = w.visibility;
            w.visibility = Window.FullScreen;
        } else {
            w.visibility = visibilityBefore === Window.FullScreen ? Window.Windowed : visibilityBefore;
        }
    }
    function poke() {
        controlsShown = true;
        hideControls.restart();
    }

    // ---------- Recap ----------
    function longDuration(seconds) {
        var s = Math.round(Number(seconds) || 0);
        var h = Math.floor(s / 3600), m = Math.floor(s % 3600 / 60), sec = s % 60;
        if (h > 0)
            return h + " h " + (m > 0 ? m + " min" : "");
        if (m > 0)
            return m + " min" + (sec > 0 ? " " + sec + " s" : "");
        return sec + " s";
    }
    function longDate(d) {
        return dayNames[d.getDay()] + " " + d.getDate() + " " + monthNames[d.getMonth()] + (d.getFullYear() === new Date().getFullYear() ? "" : " " + d.getFullYear());
    }
    function compute(list) {
        var total = 0, size = 0, fav = 0, games = {}, days = {}, hours = [], longest = null, dated = 0;
        for (var h = 0; h < 24; ++h)
            hours.push(0);
        for (var i = 0; i < list.length; ++i) {
            var c = list[i];
            var d = Number(c.duration) || 0;
            total += d;
            size += Number(c.size) || 0;
            if (Number(c.favorite) === 1)
                fav++;
            var g = c.game || "Vidéo locale";
            games[g] = (games[g] || 0) + 1;
            var t = new Date(c.created);
            if (!isNaN(t.getTime())) {
                dated++;
                var k = t.getFullYear() + "-" + t.getMonth() + "-" + t.getDate();
                if (!days[k])
                    days[k] = {
                        count: 0,
                        date: t
                    };
                days[k].count++;
                hours[t.getHours()]++;
            }
            if (!longest || d > (Number(longest.duration) || 0))
                longest = c;
        }
        var top = Object.keys(games).map(function (k) {
            return {
                name: k,
                count: games[k]
            };
        }).sort(function (a, b) {
            return b.count - a.count;
        });
        var bestDay = null;
        for (var key in days)
            if (!bestDay || days[key].count > bestDay.count)
                bestDay = days[key];
        var bestHour = 0;
        for (var j = 1; j < 24; ++j)
            if (hours[j] > hours[bestHour])
                bestHour = j;
        return {
            count: list.length,
            total: total,
            size: size,
            favorites: fav,
            games: top,
            bestDay: bestDay,
            bestHour: dated ? bestHour : -1,
            longest: longest
        };
    }
    function hourPhrase(h) {
        if (h >= 23 || h < 5)
            return "Un vrai joueur de nuit.";
        if (h >= 18)
            return "Les soirées, c'est ton moment.";
        if (h >= 12)
            return "L'après-midi, tu es en feu.";
        return "Tu joues dès le matin, respect.";
    }
    function buildSlides(s) {
        var out = [];
        out.push({
            kicker: "Ton récap ClipForge",
            big: s.count + (s.count > 1 ? " clips" : " clip"),
            small: longDuration(s.total) + " de moments gardés"
        });
        if (s.games.length) {
            var g = s.games[0];
            out.push({
                kicker: "Ton jeu n°1",
                big: g.name,
                small: g.count + (g.count > 1 ? " clips" : " clip") + " · " + Math.round(g.count * 100 / s.count) + " % de tes clips",
                list: s.games.slice(1, 3).map(function (x, i) {
                    return (i + 2) + ". " + x.name + " (" + x.count + ")";
                })
            });
        }
        if (s.bestDay)
            out.push({
                kicker: "Ton jour le plus fou",
                big: longDate(s.bestDay.date),
                small: s.bestDay.count + (s.bestDay.count > 1 ? " clips ce jour-là" : " clip ce jour-là")
            });
        if (s.bestHour >= 0)
            out.push({
                kicker: "Ton heure de gloire",
                big: s.bestHour + " h",
                small: hourPhrase(s.bestHour)
            });
        if (s.longest)
            out.push({
                kicker: "Ton plus long clip",
                big: longDuration(s.longest.duration),
                small: s.longest.name || ""
            });
        out.push({
            kicker: "Tes coups de cœur",
            big: s.favorites ? s.favorites + (s.favorites > 1 ? " favoris" : " favori") : "Aucun favori",
            small: s.favorites ? "Les clips que tu voulais vraiment garder." : "Clique sur l'étoile d'un clip pour le garder de côté."
        });
        return out;
    }

    Keys.onPressed: function (event) {
        poke();
        if (event.key === Qt.Key_Space) {
            togglePause();
        } else if (event.key === Qt.Key_Right) {
            next();
        } else if (event.key === Qt.Key_Left) {
            previous();
        } else if (event.key === Qt.Key_F || event.key === Qt.Key_F11) {
            setFullscreen(!fullscreen);
        } else if (event.key === Qt.Key_Escape) {
            close();
        } else {
            return;
        }
        event.accepted = true;
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    // ---------- Video ----------
    Item {
        anchors.fill: parent
        visible: !show.finished
        VideoOutput {
            id: output
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectFit
        }
        MediaPlayer {
            id: player
            videoOutput: output
            onMediaStatusChanged: {
                if (!show.visible || show.finished)
                    return;
                if (mediaStatus === MediaPlayer.EndOfMedia || mediaStatus === MediaPlayer.InvalidMedia)
                    show.next();
            }
        }
        PlaybackAudio {
            id: playback
            mediaPlayer: player
            volume: 1
        }
        // Title of each clip, shown for a few seconds when it starts.
        Column {
            id: title
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 48
            anchors.bottomMargin: 110
            spacing: 4
            opacity: 0
            Text {
                text: show.clip.game || "Vidéo locale"
                color: "white"
                font.pixelSize: 34
                font.weight: Font.Bold
                style: Text.Raised
                styleColor: "#80000000"
            }
            Text {
                text: {
                    var d = new Date(show.clip.created);
                    return (isNaN(d.getTime()) ? "" : show.longDate(d) + " · ") + "clip " + (show.index + 1) + " sur " + show.clips.length;
                }
                color: "#e6ffffff"
                font.pixelSize: 17
                style: Text.Raised
                styleColor: "#80000000"
            }
            SequentialAnimation {
                id: intro
                NumberAnimation {
                    target: title
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 350
                }
                PauseAnimation {
                    duration: 2600
                }
                NumberAnimation {
                    target: title
                    property: "opacity"
                    to: 0
                    duration: 600
                }
            }
        }
        // One segment per clip (stories style); a single bar when there are too many clips.
        Row {
            id: segments
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 16
            spacing: 4
            readonly property int n: Math.min(show.clips.length, 40)
            Repeater {
                model: segments.n
                delegate: Rectangle {
                    required property int index
                    width: (segments.width - segments.spacing * (segments.n - 1)) / segments.n
                    height: 4
                    radius: 2
                    color: "#4dffffff"
                    Rectangle {
                        height: parent.height
                        radius: 2
                        color: "white"
                        readonly property real local: player.duration > 0 ? player.position / player.duration : 0
                        readonly property real done: show.clips.length <= 40 ? (index < show.index ? 1 : index === show.index ? local : 0) : Math.max(0, Math.min(1, (show.index + local) / show.clips.length * segments.n - index))
                        width: parent.width * done
                    }
                }
            }
        }
    }

    // ---------- Recap slides ----------
    Rectangle {
        id: recap
        anchors.fill: parent
        visible: show.finished
        readonly property var pal: show.palettes[Math.min(show.statPage, show.palettes.length - 1) % show.palettes.length]
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: recap.pal[0]
                Behavior on color {
                    ColorAnimation {
                        duration: 500
                    }
                }
            }
            GradientStop {
                position: 1
                color: recap.pal[1]
                Behavior on color {
                    ColorAnimation {
                        duration: 500
                    }
                }
            }
        }
        Timer {
            id: statTimer
            interval: 4200
            onTriggered: show.nextStat()
        }
        Row {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 16
            spacing: 4
            visible: show.statPage < show.slides.length
            Repeater {
                model: show.slides.length
                delegate: Rectangle {
                    required property int index
                    width: (parent.width - 4 * (show.slides.length - 1)) / show.slides.length
                    height: 4
                    radius: 2
                    color: index <= show.statPage ? "white" : "#4dffffff"
                }
            }
        }
        // One big number per slide.
        Item {
            id: slide
            anchors.fill: parent
            visible: show.statPage < show.slides.length
            readonly property var s: show.slides[show.statPage] || ({})
            Column {
                id: slideBody
                anchors.centerIn: parent
                width: Math.min(parent.width - 96, 900)
                spacing: 14
                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: slide.s.kicker || ""
                    color: "#e6ffffff"
                    font.pixelSize: 22
                    font.weight: Font.DemiBold
                    font.capitalization: Font.AllUppercase
                    font.letterSpacing: 2
                }
                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: slide.s.big || ""
                    color: "white"
                    font.pixelSize: Math.min(96, show.width / 11)
                    font.weight: Font.Black
                    wrapMode: Text.Wrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                }
                Text {
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                    text: slide.s.small || ""
                    color: "white"
                    font.pixelSize: 24
                    wrapMode: Text.Wrap
                }
                Repeater {
                    model: slide.s.list || []
                    delegate: Text {
                        required property string modelData
                        width: slideBody.width
                        horizontalAlignment: Text.AlignHCenter
                        text: modelData
                        color: "#ccffffff"
                        font.pixelSize: 19
                    }
                }
            }
            ParallelAnimation {
                id: pop
                NumberAnimation {
                    target: slideBody
                    property: "scale"
                    from: 0.82
                    to: 1
                    duration: 520
                    easing.type: Easing.OutBack
                }
                NumberAnimation {
                    target: slideBody
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 380
                }
            }
            Connections {
                target: show
                function onStatPageChanged() {
                    pop.restart();
                }
                function onFinishedChanged() {
                    if (show.finished)
                        pop.restart();
                }
            }
        }
        // Summary of everything once the slides are over.
        SummaryView {
            anchors.centerIn: parent
            width: Math.min(parent.width - 64, 980)
            visible: show.statPage >= show.slides.length
            slides: show.slides
            wide: show.width > 900
            onReplayRequested: show.replay()
            onCloseRequested: show.close()
            onImageRequested: show.captureRecap(function (img) {
                appController.saveRecapImage(img);
            })
            onVideoRequested: show.captureRecap(function (img) {
                appController.exportSlideshow(show.clips, img);
            })
        }
    }

    // Same summary laid out for a 1920x1080 picture (recap image and last part of the exported video).
    Rectangle {
        id: capture
        x: -20000
        width: 1280
        height: 720
        visible: false
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0
                color: show.palettes[show.palettes.length - 1][0]
            }
            GradientStop {
                position: 1
                color: show.palettes[show.palettes.length - 1][1]
            }
        }
        SummaryView {
            anchors.centerIn: parent
            width: parent.width - 120
            slides: show.slides
            wide: true
            forImage: true
        }
        Text {
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 24
            text: "ClipForge"
            color: "#b3ffffff"
            font.pixelSize: 18
            font.weight: Font.Bold
        }
    }
    // Used by the screenshot test: video exports the first `limit` clips.
    function exportRecap(video, limit) {
        captureRecap(function (img) {
            if (video)
                appController.exportSlideshow(limit ? show.clips.slice(0, limit) : show.clips, img);
            else
                appController.saveRecapImage(img);
        });
    }
    function captureRecap(done) {
        capture.visible = true;
        capture.grabToImage(function (result) {
            capture.visible = false;
            done(result.image);
        }, Qt.size(1920, 1080));
    }

    component SummaryView: ColumnLayout {
        id: summary
        property var slides: []
        property bool wide: true
        property bool forImage: false
        signal replayRequested
        signal closeRequested
        signal imageRequested
        signal videoRequested
        spacing: 22
        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: "Ton récap"
            color: "white"
            font.pixelSize: 44
            font.weight: Font.Black
        }
        GridLayout {
            Layout.fillWidth: true
            columns: summary.wide ? 3 : 2
            rowSpacing: 14
            columnSpacing: 14
            Repeater {
                model: summary.slides
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 130
                    radius: 16
                    color: "#33000000"
                    border.color: "#40ffffff"
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 4
                        Text {
                            Layout.fillWidth: true
                            text: modelData.kicker
                            color: "#ccffffff"
                            font.pixelSize: 13
                            font.capitalization: Font.AllUppercase
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: modelData.big
                            color: "white"
                            font.pixelSize: 26
                            font.weight: Font.Bold
                            elide: Text.ElideRight
                        }
                        Text {
                            Layout.fillWidth: true
                            text: modelData.small
                            color: "#e6ffffff"
                            font.pixelSize: 13
                            wrapMode: Text.Wrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
        Flow {
            Layout.fillWidth: true
            Layout.preferredHeight: implicitHeight
            visible: !summary.forImage
            spacing: 12
            // Flow cannot center itself: the padding does it.
            leftPadding: Math.max(0, (width - buttonsWidth) / 2)
            readonly property real buttonsWidth: {
                var w = 0;
                for (var i = 0; i < children.length; ++i)
                    if (children[i].visible)
                        w += children[i].width + spacing;
                return Math.max(0, w - spacing);
            }
            ActionButton {
                text: "Revoir le diaporama"
                onClicked: summary.replayRequested()
            }
            ActionButton {
                text: "Enregistrer l'image"
                onClicked: summary.imageRequested()
            }
            ActionButton {
                text: appController.slideshowBusy ? "Export vidéo… " + Math.round(appController.slideshowProgress * 100) + " %" : "Exporter en vidéo"
                enabled: !appController.slideshowBusy
                onClicked: summary.videoRequested()
            }
            ActionButton {
                visible: appController.slideshowBusy
                text: "Annuler l'export"
                onClicked: appController.cancelSlideshowExport()
            }
            ActionButton {
                text: "Fermer"
                primary: true
                onClicked: summary.closeRequested()
            }
        }
    }

    // Clicks: on the video they pause, on the recap they go to the next slide. Moving the mouse shows the controls.
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: show.controlsShown ? Qt.ArrowCursor : Qt.BlankCursor
        enabled: !(show.finished && show.statPage >= show.slides.length)
        onPositionChanged: show.poke()
        onClicked: show.finished ? show.nextStat() : show.togglePause()
        onDoubleClicked: show.setFullscreen(!show.fullscreen)
    }
    Timer {
        id: hideControls
        interval: 2500
        onTriggered: show.controlsShown = false
    }

    // ---------- Controls ----------
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 28
        width: bar.implicitWidth + 24
        height: bar.implicitHeight + 16
        radius: 14
        color: "#cc101119"
        border.color: "#30ffffff"
        opacity: show.controlsShown && !(show.finished && show.statPage >= show.slides.length) ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: 200
            }
        }
        HoverHandler {
            onHoveredChanged: if (hovered)
                hideControls.stop()
            else
                show.poke()
        }
        RowLayout {
            id: bar
            anchors.centerIn: parent
            spacing: 8
            ActionButton {
                text: "Précédent"
                onClicked: show.previous()
            }
            ActionButton {
                visible: !show.finished
                text: show.paused ? "Lecture" : "Pause"
                primary: true
                onClicked: show.togglePause()
            }
            ActionButton {
                text: show.finished ? "Suivant" : (show.index + 1 < show.clips.length ? "Clip suivant" : "Voir le récap")
                onClicked: show.next()
            }
            ActionButton {
                visible: !show.finished
                text: "Passer au récap"
                onClicked: show.end()
            }
            ActionButton {
                text: show.fullscreen ? "Quitter le plein écran" : "Plein écran"
                onClicked: show.setFullscreen(!show.fullscreen)
            }
            ActionButton {
                text: "Fermer"
                onClicked: show.close()
            }
        }
    }
}
