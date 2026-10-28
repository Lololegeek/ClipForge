pragma Singleton
import QtQuick

QtObject {
    readonly property color canvas: "#101119"
    readonly property color navigation: "#171822"
    readonly property color surface: "#20222f"
    readonly property color raised: "#292c3c"
    readonly property color line: "#36394d"
    readonly property color accent: "#8c8cff"
    readonly property color accentHover: "#a3a3ff"
    readonly property color accentSurface: "#30304c"
    readonly property color text: "#f0f1fa"
    readonly property color secondary: "#b0b3c8"
    readonly property color success: "#81d9b1"
    readonly property color danger: "#ffadb5"
    readonly property string font: "Segoe UI"
    readonly property string mono: "Consolas"
    readonly property int radius: 12
    readonly property int radiusSm: 8
    // Plain-language label for a capture mode stored in the config.
    function sourceLabel(mode) {
        return ({ "display": "Écran entier", "window": "Fenêtre", "game": "Jeu" })[mode] || "Écran entier";
    }

    // 75 -> "1:15", 3725 -> "1:02:05"; tenths optional for editor timecodes.
    function duration(seconds, tenths) {
        var s = Math.max(0, Number(seconds) || 0);
        var whole = Math.floor(s);
        var h = Math.floor(whole / 3600), m = Math.floor(whole % 3600 / 60), sec = whole % 60;
        var text = (h > 0 ? h + ":" + String(m).padStart(2, "0") : m) + ":" + String(sec).padStart(2, "0");
        return tenths ? text + "." + Math.floor((s - whole) * 10) : text;
    }
    function size(bytes) {
        var b = Number(bytes) || 0;
        if (b >= 1073741824)
            return (b / 1073741824).toFixed(1) + " Go";
        if (b >= 1048576)
            return Math.round(b / 1048576) + " Mo";
        return Math.max(1, Math.round(b / 1024)) + " Ko";
    }
    function date(iso) {
        var d = new Date(iso);
        if (isNaN(d.getTime()))
            return "";
        var today = new Date();
        today.setHours(0, 0, 0, 0);
        var days = Math.round((today - new Date(d.getFullYear(), d.getMonth(), d.getDate())) / 86400000);
        var time = Qt.formatTime(d, "HH:mm");
        if (days === 0)
            return "Aujourd'hui " + time;
        if (days === 1)
            return "Hier " + time;
        return Qt.formatDate(d, days < 300 ? "d MMM" : "d MMM yyyy") + " " + time;
    }
}
