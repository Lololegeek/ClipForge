#include "editor/EditProject.h"
#include "export/MediaJobs.h"
#include "library/ClipModel.h"
#include "settings/Settings.h"
#include <QFile>
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
        auto a = cf::MediaJobs::exportArguments(p, "out.mp4", 720, 60, "h264");
        auto graph = a[a.indexOf("-filter_complex") + 1];
        QVERIFY(graph.contains("atempo=0.5,atempo=0.5"));
        QVERIFY(graph.contains("atempo=2,atempo=2"));
        QVERIFY(graph.contains("concat=n=2:v=1:a=1"));
        QVERIFY(a.contains(p.source));
        p.hasAudio = false;
        a = cf::MediaJobs::exportArguments(p, "out.mp4", 0, 30, "h264");
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
            int id = model.get(0)["id"].toInt();
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
