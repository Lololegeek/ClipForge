#include "Controller.h"
#include "Overlay.h"
#include "platform/WindowCatalog.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QPainter>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QScreen>
#include <QStorageInfo>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
#include <windows.h>
namespace cf {
void Controller::start() {
    if (!ready_ || !(state_ == "Idle" || state_ == "Error"))
        return;
    state_ = "Initializing";
    emit changed();
    auto s = settings_;
    if (s.native()) {
        // 0×0 = follow the screen: its real pixel size, made even and kept within what the encoders accept.
        const QScreen *screen = QGuiApplication::primaryScreen();
        const QSize px = screen ? (screen->size() * screen->devicePixelRatio()) : QSize(1920, 1080);
        s.width = std::clamp(px.width() & ~1, 320, 7680);
        s.height = std::clamp(px.height() & ~1, 180, 4320);
    }
    QMetaObject::invokeMethod(engine_, [this, s] { engine_->start(s); });
}
void Controller::stop() {
    QMetaObject::invokeMethod(engine_, &ObsEngine::stop);
}
void Controller::saveReplay() {
    requestClip(settings_.clipSeconds);
}
void Controller::saveFull() {
    requestClip(0);
}
void Controller::requestClip(double seconds) {
    if (state_ != "Buffering" && state_ != "Recording" && state_ != "Saving") {
        notify("Activez d'abord l'Instant Replay.", true);
        toast("Replay inactif", "Activez l'Instant Replay pour clipper.", Overlay::Error);
        return;
    }
    // One clip at a time and at most one request per 1.5 s: mashing the key must not queue dozens of files.
    if (state_ == "Saving" || !gate(saveGate_, 1500))
        return;
    const bool shortened = seconds > 0 && seconds < settings_.replaySeconds;
    pendingClips_ << PendingClip{shortened ? seconds : 0, QDateTime::currentMSecsSinceEpoch(), currentGameName()};
    toast("Clip en cours…",
          shortened ? QString("Sauvegarde des %1 dernières secondes").arg(int(seconds)) : "Sauvegarde de tout le tampon",
          Overlay::Progress);
    QMetaObject::invokeMethod(engine_, &ObsEngine::saveReplay);
}
// Applies the file name template and the per-game folder; returns the final path (unchanged on any failure).
QString Controller::organizeClipFile(const QString &path, const QString &gameName) {
    const auto &s = settings_;
    const QString game = gameName.isEmpty() ? QString("Clip") : gameName;
    if (s.fileTemplate.trimmed().isEmpty() && !(s.gameFolders && !gameName.isEmpty()))
        return path;
    auto clean = [](QString v) {
        v.replace(": ", " - "); // "Firefighting Simulator: Ignite" -> "Firefighting Simulator - Ignite"
        for (const QChar c : QStringLiteral("<>:\"/\\|?*"))
            v.replace(c, '-');
        return v.trimmed();
    };
    const auto now = QDateTime::currentDateTime();
    QString name = s.fileTemplate.trimmed();
    if (name.isEmpty())
        name = QFileInfo(path).completeBaseName();
    else {
        name.replace("{jeu}", game);
        name.replace("{date}", now.toString("yyyy-MM-dd"));
        name.replace("{heure}", now.toString("HH-mm-ss"));
    }
    name = clean(name);
    if (name.isEmpty())
        return path;
    QDir dir(QFileInfo(path).absolutePath());
    if (s.gameFolders && !gameName.isEmpty()) {
        dir = QDir(s.clips + "/" + clean(gameName));
        if (!dir.mkpath(".")) {
            notify("Dossier du jeu impossible à créer, le clip reste dans le dossier principal.", true);
            dir = QDir(QFileInfo(path).absolutePath());
        }
    }
    const QString ext = QFileInfo(path).suffix();
    QString target = dir.filePath(name + "." + ext);
    for (int i = 2; QFileInfo::exists(target) && QFileInfo(target) != QFileInfo(path); ++i)
        target = dir.filePath(QString("%1 (%2).%3").arg(name).arg(i).arg(ext));
    if (QFileInfo(target) == QFileInfo(path))
        return path;
    if (!QFile::rename(path, target)) {
        notify("Renommage ou rangement du clip impossible, le nom d'origine est conservé.", true);
        return path;
    }
    return target;
}
void Controller::clipSaved(QString p) {
    if (p.endsWith(".mkv", Qt::CaseInsensitive)) {
        // Manual recordings (self-test) are kept whole and carry no markers.
        lastClip_ = p;
        jobs_.import(p, game_, true);
        emit changed();
        return;
    }
    const auto clip = pendingClips_.isEmpty() ? PendingClip{0, QDateTime::currentMSecsSinceEpoch(), game_}
                                              : pendingClips_.takeFirst();
    p = organizeClipFile(p, clip.game);
    const double window = clip.keep > 0 ? clip.keep : settings_.replaySeconds;
    QVariantList offsets;
    for (auto m : markers_)
        if (m <= clip.at && clip.at - m <= window * 1000)
            offsets << double(clip.at - m) / 1000.0;
    clipMarkers_[QFileInfo(p).absoluteFilePath()] = offsets;
    if (settings_.saveSound)
        MessageBeep(MB_OK);
    notify("Clip sauvegardé : " + QFileInfo(p).fileName() + " (cliquez pour l'ouvrir)");
    notifyClip_ = p;
    toast("Clip sauvegardé", QFileInfo(p).fileName(), Overlay::Success);
    if (clip.keep > 0) {
        clipGames_[p] = clip.game;
        jobs_.trimTail(p, clip.keep);
        return;
    }
    lastClip_ = p;
    jobs_.import(p, clip.game, true);
    emit changed();
}
void Controller::clipImported(QVariantMap data) {
    const auto it = clipMarkers_.find(QFileInfo(data["path"].toString()).absoluteFilePath());
    if (it == clipMarkers_.end()) {
        library_.upsert(data);
        return;
    }
    // Markers were measured back from the key press, which is the end of the saved file.
    const double duration = data["duration"].toDouble();
    QJsonArray markers;
    for (const auto &offset : *it) {
        const double at = duration - offset.toDouble();
        if (at >= 0)
            markers << std::round(at * 100) / 100;
    }
    clipMarkers_.erase(it);
    if (!markers.isEmpty())
        data["markers"] = QString::fromUtf8(QJsonDocument(markers).toJson(QJsonDocument::Compact));
    library_.upsert(data);
    cleanupLibrary();
}
void Controller::addMarker() {
    if (state_ != "Buffering" && state_ != "Recording" && state_ != "Saving") {
        toast("Replay inactif", "Activez l'Instant Replay pour poser un marqueur.", Overlay::Error);
        return;
    }
    const auto now = QDateTime::currentMSecsSinceEpoch();
    markers_.removeIf([&](qint64 m) { return now - m > (settings_.replaySeconds + 10) * 1000LL; });
    markers_ << now;
    toast("Marqueur posé", "Il apparaîtra sur la timeline du prochain clip", Overlay::Info);
}
void Controller::setMicMuted(bool muted) {
    micMuted_ = muted;
    QMetaObject::invokeMethod(engine_, [this, muted] { engine_->setMicMuted(muted); });
    emit changed();
}
void Controller::micKey() {
    if (settings_.microphone == "disabled")
        return;
    if (settings_.pushToTalk) {
        setMicMuted(false);
        pttTimer_.start();
        return;
    }
    setMicMuted(!micMuted_);
    toast(micMuted_ ? "Micro coupé" : "Micro actif", {}, micMuted_ ? Overlay::Error : Overlay::Info);
}
void Controller::configureHotkeys() {
    auto e = hotkeys_.configure(settings_.saveKey, settings_.markerKey, settings_.micKey);
    if (!e.isEmpty())
        notify(e, true);
    updateTrayState();
}
int Controller::cleanupLibrary() {
    const int removed = library_.cleanup(qint64(settings_.cleanupGB) * 1073741824LL, settings_.cleanupDays);
    if (removed > 0)
        notify(QString("Nettoyage : %1 ancien(s) clip(s) envoyé(s) à la corbeille.").arg(removed));
    return removed;
}
void Controller::cleanupNow() {
    if (settings_.cleanupGB <= 0 && settings_.cleanupDays <= 0) {
        notify("Aucune limite de nettoyage n'est définie.", true);
        return;
    }
    if (cleanupLibrary() == 0)
        notify("Rien à nettoyer : la bibliothèque respecte les limites.");
}
void Controller::toggleRecording() {
    if (!gate(recordGate_, 1500))
        return;
    if (state_ == "Idle" || state_ == "Error")
        start();
    QMetaObject::invokeMethod(engine_, &ObsEngine::toggleRecording);
}
// Options that only affect the app never need the capture pipeline to be rebuilt.
static QJsonObject captureKey(const Settings &x) {
    auto o = x.json();
    for (const auto &k : Settings::appKeys())
        o.remove(k);
    o.remove("clipSeconds");
    o["clips"] = x.clips; // the replay writes there: a new folder needs a restart
    o["temporary"] = x.temporary;
    return o;
}
void Controller::useSettings(const Settings &next, const QString &why) {
    const bool restart = state_ == "Buffering" && captureKey(next) != captureKey(settings_);
    settings_ = next;
    emit changed();
    if (restart) {
        restartPending_ = true;
        notify(why);
        stop();
    }
}
bool Controller::apply(QVariantMap v) {
    const bool running = state_ == "Buffering" || state_ == "Recording";
    if (!running && state_ != "Idle" && state_ != "Error") {
        notify("Patientez un instant : la capture est occupée (démarrage ou sauvegarde).", true);
        return false;
    }
    auto saved = Settings::fromJson(QJsonObject::fromVariantMap(v));
    QString e;
    if (!saved.save(data_ + "/settings.json", &e)) {
        notify(e, true);
        return false;
    }
    // While a game profile is active the page edits the global settings; the profile stays on top of them.
    if (!profileExe_.isEmpty())
        global_ = saved;
    const auto s = profileExe_.isEmpty() ? saved : withProfile(saved, saved.profiles.value(profileExe_).toObject());
    const bool restart = running && captureKey(s) != captureKey(settings_);
    const bool micChanged = s.pushToTalk != settings_.pushToTalk || s.micKey != settings_.micKey;
    settings_ = s;
    emit configChanged();
    e = hotkeys_.configure(s.saveKey, s.markerKey, s.micKey);
    QTimer::singleShot(0, this, &Controller::updateTrayState);
    if (!e.isEmpty())
        notify(e, true);
    else
        notify("Réglages enregistrés.");
    if (micChanged && running) {
        pttTimer_.stop();
        setMicMuted(s.pushToTalk);
    }
    cleanupLibrary();
    emit changed();
    if (restart) {
        // Rebuild the replay live: stop it, and the Idle transition below starts it again with the new values.
        restartPending_ = true;
        notify("Réglages enregistrés : redémarrage du replay…");
        stop();
        return true;
    }
    // Idle/Error: bring the replay straight back with the new values.
    if (!running && settings_.alwaysReplay && !test_ && !uiOnly_)
        QTimer::singleShot(150, this, &Controller::start);
    return true;
}
bool Controller::allowClose() const {
    return quitting_ || !settings_.closeToTray;
}
void Controller::quit() {
    if (jobs_.busy() || state_ == "Saving") {
        notify("Attendez la fin de la sauvegarde ou de l'export avant de quitter.", true);
        return;
    }
    // Never wait forever for the engine to go idle: a stuck state must not keep the process alive.
    if (state_ != "Idle" && state_ != "Error" && quitAttempts_++ < 15) {
        stop();
        QTimer::singleShot(400, this, &Controller::quit);
        return;
    }
    quitting_ = true;
    emit changed();
    updater_.launchInstaller();
    QApplication::quit();
}
static const char *kRunKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
bool Controller::autostart() const {
    return QSettings(kRunKey, QSettings::NativeFormat).contains("ClipForge");
}
void Controller::setAutostart(bool enabled) {
    QSettings run(kRunKey, QSettings::NativeFormat);
    if (enabled)
        run.setValue("ClipForge", "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\" --headless");
    else
        run.remove("ClipForge");
    emit changed();
}
void Controller::checkForUpdates() {
    updater_.check(true, false);
}
void Controller::installUpdate() {
    if (updater_.state() == "available")
        updater_.downloadNow();
    else if (updater_.ready())
        quit();
    else
        updater_.check(true, true);
}
void Controller::runSelfTest() {
    int duration = 32000;
    auto args = QCoreApplication::arguments();
    int i = args.indexOf("--test-seconds");
    if (i >= 0 && i + 1 < args.size())
        duration = std::clamp(args[i + 1].toInt(), 32, 3600) * 1000;
    if (duration > 60000)
        settings_.replaySeconds = 60;
    if (args.contains("--test-1080")) {
        settings_.width = 1920;
        settings_.height = 1080;
        settings_.fps = 60;
        settings_.bitrate = 16000;
    }
    if (args.contains("--software-test"))
        settings_.encoder = "obs_x264";
    if (args.contains("--window-test")) {
        settings_.mode = "window";
        settings_.target = "ClipForge Capture Fixture:ClipForgeFixture:ClipForgeFixture.exe";
        settings_.audio = "disabled";
    }
    start();
    QTimer::singleShot(duration - 14000, this, &Controller::saveReplay);
    QTimer::singleShot(duration - 13800, this, &Controller::saveReplay);
    // Exercise the rapid-keypress guard, then save again after its cooldown.
    QTimer::singleShot(duration - 12000, this, &Controller::saveReplay);
    QTimer::singleShot(duration - 10000, this, &Controller::toggleRecording);
    QTimer::singleShot(duration - 5000, this, &Controller::toggleRecording);
    QTimer::singleShot(duration - 3000, this, &Controller::stop);
    QTimer::singleShot(duration, this, [this] {
        QFile f(data_ + "/metrics.json");
        if (f.open(QIODevice::WriteOnly))
            f.write(QJsonDocument(QJsonObject::fromVariantMap(metrics_)).toJson());
        QFile samples(data_ + "/samples.json");
        if (samples.open(QIODevice::WriteOnly))
            samples.write(QJsonDocument(testSamples_).toJson());
        quitting_ = true;
        QApplication::quit();
    });
}
} // namespace cf
namespace cf {
// Friendly program name from the exe's version resource (ProductName, else FileDescription); empty if absent.
static QString productName(const QString &path) {
    const auto wpath = QDir::toNativeSeparators(path).toStdWString();
    DWORD handle = 0;
    const DWORD size = GetFileVersionInfoSizeW(wpath.c_str(), &handle);
    if (!size)
        return {};
    QByteArray data(int(size), 0);
    if (!GetFileVersionInfoW(wpath.c_str(), 0, size, data.data()))
        return {};
    struct LangCp {
        WORD lang, cp;
    } *tr = nullptr;
    UINT len = 0;
    QList<QString> prefixes;
    if (VerQueryValueW(data.data(), LR"(\VarFileInfo\Translation)", reinterpret_cast<LPVOID *>(&tr), &len) && tr &&
        len >= sizeof(LangCp))
        prefixes << QString("%1%2").arg(tr->lang, 4, 16, QChar('0')).arg(tr->cp, 4, 16, QChar('0'));
    prefixes << "040904b0" << "040c04b0" << "040904e4";
    for (const char *field : {"ProductName", "FileDescription"})
        for (const QString &pre : prefixes) {
            const auto key = QString(R"(\StringFileInfo\%1\%2)").arg(pre, field).toStdWString();
            wchar_t *val = nullptr;
            UINT vlen = 0;
            if (VerQueryValueW(data.data(), key.c_str(), reinterpret_cast<LPVOID *>(&val), &vlen) && val && vlen > 1) {
                const QString v = QString::fromWCharArray(val).trimmed();
                if (!v.isEmpty() && !v.contains("Unreal", Qt::CaseInsensitive) && !v.contains("Unity", Qt::CaseInsensitive) &&
                    v.compare("Shipping", Qt::CaseInsensitive) != 0)
                    return v;
            }
        }
    return {};
}
// Store/Game Pass games (WindowsApps, XboxGames): the title from MicrosoftGame.config or appxmanifest.xml found
// in one of the exe's parent folders, e.g. "Firefighting Simulator: Ignite". Empty if none.
static QString packageName(const QString &path) {
    static const QRegularExpression gameConfig(R"re(DefaultDisplayName\s*=\s*"([^"]+)")re");
    static const QRegularExpression manifest(R"re(<Properties>[\s\S]*?<DisplayName>([^<]+)</DisplayName>)re");
    QDir dir = QFileInfo(path).absoluteDir();
    for (int depth = 0; depth < 5; ++depth) {
        for (const auto &[file, re] : {std::pair{"MicrosoftGame.config", &gameConfig}, std::pair{"appxmanifest.xml", &manifest}}) {
            QFile f(dir.filePath(file));
            if (!f.open(QIODevice::ReadOnly))
                continue;
            const auto m = re->match(QString::fromUtf8(f.read(256 * 1024)));
            const QString v = m.hasMatch() ? m.captured(1).trimmed() : QString();
            if (!v.isEmpty() && !v.startsWith("ms-resource:", Qt::CaseInsensitive))
                return v;
        }
        if (!dir.cdUp())
            break;
    }
    return {};
}
// True when the exe sits in an Xbox/Game Pass game package (MicrosoftGame.config in a parent folder).
static bool hasGameConfig(const QString &path) {
    QDir dir = QFileInfo(path).absoluteDir();
    for (int depth = 0; depth < 6; ++depth) {
        if (QFileInfo::exists(dir.filePath("MicrosoftGame.config")))
            return true;
        if (!dir.cdUp())
            break;
    }
    return false;
}
// Guess whether a running program is a game (launcher folders, Game Pass package, fullscreen) and not a tool.
bool Controller::looksLikeGame(const QString &path, bool fullscreen) {
    const QString p = QDir::fromNativeSeparators(path).toLower();
    const QString exe = QFileInfo(path).completeBaseName().toLower();
    static const QStringList tools{"explorer", "chrome", "msedge", "firefox", "zen", "brave", "opera", "vivaldi", "code",
                                   "devenv", "discord", "spotify", "obs64", "slack", "teams", "notepad", "taskmgr",
                                   "windowsterminal", "cmd", "powershell", "raycast", "powertoys", "steam", "epicgameslauncher",
                                   "xboxpcapp", "gamingservices", "applicationframehost", "textinputhost", "searchhost"};
    for (const auto &t : tools)
        if (exe.startsWith(t))
            return false;
    static const QStringList folders{"/steamapps/", "/xboxgames/", "/epic games/", "/gog galaxy/games/", "/gog games/",
                                     "/riot games/", "/battle.net/", "/ubisoft game launcher/games/", "/ea games/",
                                     "/origin games/", "/rockstar games/"};
    for (const auto &f : folders)
        if (p.contains(f))
            return true;
    return hasGameConfig(path) || fullscreen;
}
// Game name for a clip: the active profile's name, otherwise the foreground program when the key is pressed.
QString Controller::currentGameName() const {
    if (!game_.isEmpty())
        return game_;
    const auto path = foregroundExecutable();
    if (path.isEmpty())
        return {};
    auto name = QFileInfo(path).completeBaseName();
    const auto lower = name.toLower();
    if (lower == QFileInfo(QCoreApplication::applicationFilePath()).completeBaseName().toLower() ||
        lower == "explorer" || lower == "searchhost" || lower == "applicationframehost" || lower == "shellexperiencehost")
        return {};
    return programTitle(path);
}
// Readable name of a program: store/Game Pass title, else the exe's product name, else the cleaned file name.
QString Controller::programTitle(const QString &path) {
    if (const QString pkg = packageName(path); !pkg.isEmpty())
        return pkg;
    const QString product = productName(path);
    if (!product.isEmpty())
        return product;
    // Typical Unreal/engine suffixes: "FFSIgnite-WinGDK-Shipping" -> "FFSIgnite".
    static const QRegularExpression suffixes(R"((?:[-_ ](?:Win(?:64|GDK|32)|Shipping|Game|x64|x86|64|Binaries))+$)",
                                              QRegularExpression::CaseInsensitiveOption);
    QString name = QFileInfo(path).completeBaseName();
    const QString cleaned = QString(name).remove(suffixes);
    return cleaned.isEmpty() ? name : cleaned;
}
} // namespace cf
