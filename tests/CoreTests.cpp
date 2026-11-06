#include "editor/EditProject.h"
#include "export/MediaJobs.h"
#include "library/ClipModel.h"
#include "hotkeys/GlobalHotkeys.h"
#include "settings/Settings.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>
class CoreTests : public QObject {
    Q_OBJECT
  private slots:
    void settingsValidation() {
        auto s = cf::Settings::defaults();
        QVERIFY(s.validate().isEmpty());
        s.width = 1919;
        QVERIFY(!s.validate().isEmpty());
        s.width = 1920;
        s.fps = 59;
        QVERIFY(!s.validate().isEmpty());
        s.fps = 60;
        s.saveKey = 0x10;
        QVERIFY(!s.validate().isEmpty());
    }
    void atomicSettings() {
        QTemporaryDir d;
        auto s = cf::Settings::defaults();
        s.clips = d.path();
        s.replaySeconds = 600;
        QString p = d.filePath("settings.json");
        QVERIFY(s.save(p));
        QCOMPARE(cf::Settings::load(p).replaySeconds, 600);
        s.width = 1;
        QVERIFY(!s.save(p));
        QCOMPARE(cf::Settings::load(p).width, 1920);
    }
    void hotkeyCodes() {
        using namespace cf::hotkey;
        QVERIFY(valid(0x77));                                // plain F8 (old settings files)
        QVERIFY(!valid(0x41));                               // bare "A" would hijack typing
        QVERIFY(valid(make('K', kCtrl | kAlt)));
        QVERIFY(!valid(make(0x10, kCtrl)));                  // a modifier alone is not a shortcut
        QCOMPARE(name(0), QString("Désactivée"));
        QCOMPARE(name(0x77), QString("F8"));
        QCOMPARE(name(make('K', kCtrl | kShift)), QString("Ctrl+Maj+K"));
        cf::Settings s = cf::Settings::defaults();
        s.saveKey = make(0x77, kShift);
        QVERIFY(!s.validate().isEmpty());                    // Shift+save is the whole-buffer key
        s.saveKey = make('G', kCtrl);
        s.markerKey = make('G', kCtrl | kShift);
        QVERIFY(!s.validate().isEmpty());
        s.markerKey = make('H', kCtrl);
        QVERIFY(s.validate().isEmpty());
    }
    void nativeResolution() {
        cf::Settings s = cf::Settings::defaults();
        s.width = 0;
        s.height = 0;
        QVERIFY(s.native());
        QVERIFY(s.validate().isEmpty());
        s.width = 1920; // half-native is not a thing
        QVERIFY(!s.validate().isEmpty());
        s.height = 1080;
        s.width = 0;
        s.repair();
        QVERIFY(!s.native()); // repair resets the inconsistent pair to the defaults
    }
    void wrongTypesKeepDefaults() {
        auto s = cf::Settings::fromJson(QJsonDocument::fromJson(
            R"({"systemVolume":"loud","micVolume":null,"fps":"x","replaySeconds":[1],"gameFolders":"yes"})").object());
        const auto d = cf::Settings::defaults();
        QCOMPARE(s.systemVolume, d.systemVolume);
        QCOMPARE(s.micVolume, d.micVolume);
        QCOMPARE(s.fps, d.fps);
        QCOMPARE(s.replaySeconds, d.replaySeconds);
        QCOMPARE(s.gameFolders, d.gameFolders);
    }
    void qtKeyMapping() {
        QCOMPARE(cf::hotkey::fromQt(Qt::Key_F9, 0), 0x78);
        QCOMPARE(cf::hotkey::fromQt(Qt::Key_X, 0), int('X'));
        QCOMPARE(cf::hotkey::fromQt(Qt::Key_Control, 0), 0);
        QVERIFY(cf::hotkey::valid(cf::hotkey::make(cf::hotkey::fromQt(Qt::Key_X, 0), cf::hotkey::kCtrl)));
        QVERIFY(!cf::hotkey::valid(cf::hotkey::make(cf::hotkey::fromQt(Qt::Key_X, 0), 0)));
    }
    void lenientSettings() {
        // One broken field resets only itself: the rest of the user's settings survive.
        QTemporaryDir d;
        QString p = d.filePath("settings.json");
        QFile f(p);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QString(R"({"clips":"%1","replaySeconds":120,"fps":75,"codec":"vp9","bitrate":24000,"micKey":119})")
                    .arg(d.path())
                    .toUtf8());
        f.close();
        auto s = cf::Settings::load(p);
        QVERIFY(s.validate().isEmpty());
        QCOMPARE(s.clips, d.path());
        QCOMPARE(s.replaySeconds, 120);
        QCOMPARE(s.bitrate, 24000);
        QCOMPARE(s.fps, 60);
        QCOMPARE(s.codec, QString("h264"));
        QCOMPARE(s.micKey, 0); // clashed with the save key (F8)
    }
    void editRoundtrip() {
        QTemporaryDir d;
        QFile f(d.filePath("source.mp4"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("test");
        f.close();
        cf::EditProject p;
        p.source = f.fileName();
        p.segments = {{1, 4, 0.25}, {8, 10, 4}};
        QVERIFY(p.validate().isEmpty());
        QVERIFY(p.save(d.filePath("project.clipforge")));
        auto read = cf::EditProject::load(d.filePath("project.clipforge"));
        QCOMPARE(read.segments.size(), 2);
        QCOMPARE(read.segments[1].speed, 4.0);
        read.segments[0].out = 0;
        QVERIFY(!read.validate().isEmpty());
    }
    void exportGraph() {
        cf::EditProject p;
        p.source = "a path/source.mp4";
        p.segments = {{1, 4, 0.25}, {5, 8, 4}};
        cf::ExportOptions o;
        o.height = 720;
        auto a = cf::MediaJobs::exportArguments(p, "out.mp4", o);
        auto graph = a[a.indexOf("-filter_complex") + 1];
        QVERIFY(graph.contains("atempo=0.5,atempo=0.5"));
        QVERIFY(graph.contains("atempo=2,atempo=2"));
        QVERIFY(graph.contains("concat=n=2:v=1:a=1"));
        QVERIFY(a.contains(p.source));
        QVERIFY(graph.contains("scale=-2:720"));
        // Separate game/microphone tracks are remixed with their own levels.
        p.audioTracks = 3;
        p.micVolume = 0.5;
        graph = cf::MediaJobs::exportArguments(p, "out.mp4", o)[a.indexOf("-filter_complex") + 1];
        QVERIFY(graph.contains("[0:a:1]") && graph.contains("[0:a:2]") && graph.contains("volume=0.5"));
        // Crop and 9:16 are applied once, after the concatenation.
        p.cropX = 0.25;
        p.cropW = 0.5;
        o.vertical = true;
        graph = cf::MediaJobs::exportArguments(p, "out.mp4", o)[a.indexOf("-filter_complex") + 1];
        QVERIFY(graph.contains("[c]null,crop=") && graph.contains("ih*9/16"));
        // A size cap switches to a computed bitrate.
        o = {};
        o.maxMB = 10;
        a = cf::MediaJobs::exportArguments(p, "out.mp4", o);
        QVERIFY(a.contains("-b:v") && !a.contains("-crf"));
        // Animated formats drop the audio.
        o = {};
        o.format = "gif";
        a = cf::MediaJobs::exportArguments(p, "out.gif", o);
        QVERIFY(a[a.indexOf("-filter_complex") + 1].contains("paletteuse") && !a.contains("[a]"));
        o.format = "webp";
        QVERIFY(cf::MediaJobs::exportArguments(p, "out.webp", o).contains("libwebp_anim"));
        p.hasAudio = false;
        o = {};
        o.fps = 30;
        a = cf::MediaJobs::exportArguments(p, "out.mp4", o);
        QVERIFY(!a[a.indexOf("-filter_complex") + 1].contains("[0:a"));
    }
    void libraryPersistence() {
        QTemporaryDir d;
        QString db = d.filePath("library.sqlite"), video = d.filePath("clip.mp4");
        QFile f(video);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("bytes");
        f.close();
        {
            cf::ClipModel model(db);
            QVERIFY(model.isOpen());
            model.upsert({{"path", video},
                          {"name", "Victory"},
                          {"duration", 15.0},
                          {"size", 5},
                          {"created", "2026-10-18"},
                          {"game", "Test"}});
            QCOMPARE(model.count(), 1);
            QCOMPARE(model.gameList().size(), 1);
            model.filterGame("Other");
            QCOMPARE(model.count(), 0);
            model.filterGame("*");
            QCOMPARE(model.count(), 1);
            int id = model.get(0)["id"].toInt();
            QCOMPARE(model.tagMany({id}, "ace"), 1);
            QVERIFY(model.get(0)["tags"].toString().contains("ace"));
            QVERIFY(model.favorite(id, true));
            QVERIFY(model.tags(id, "finale"));
            model.filter("finale", "created", true);
            QCOMPARE(model.count(), 1);
            model.filter("missing");
            QCOMPARE(model.count(), 0);
            model.filter("%");
            QCOMPARE(model.count(), 0);
            model.filter("");
            QVERIFY(model.rename(id, "Victory_final"));
            QCOMPARE(model.get(0)["name"].toString(), QString("Victory_final"));
            model.filter("y_f");
            QCOMPARE(model.count(), 1);
            model.filter("");
            QVERIFY(model.favorite(id, false));
            QCOMPARE(model.data(model.index(0), cf::ClipModel::Favorite).toBool(), false);
        }
        {
            cf::ClipModel model(db);
            QCOMPARE(model.count(), 1);
            video = model.get(0)["path"].toString();
            QVERIFY(QFile::remove(video));
            model.reload();
            QVERIFY(model.data(model.index(0), cf::ClipModel::Missing).toBool());
            QVERIFY(model.remove(model.get(0)["id"].toInt()));
            QCOMPARE(model.count(), 0);
        }
    }
};
QTEST_GUILESS_MAIN(CoreTests)
#include "CoreTests.moc"
