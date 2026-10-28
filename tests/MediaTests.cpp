#include "export/MediaJobs.h"
#include "hotkeys/GlobalHotkeys.h"
#include "playback/PlaybackAudio.h"
#include <QAudioDecoder>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMediaPlayer>
#include <QProcess>
#include <QVideoFrame>
#include <QVideoSink>
#include <QtTest>
#include <windows.h>
class MediaTests : public QObject {
    Q_OBJECT
    QString source_, tools_, output_;
  private slots:
    void initTestCase() {
        source_ = qEnvironmentVariable("CLIPFORGE_TEST_VIDEO");
        tools_ = QCoreApplication::applicationDirPath() + "/tools";
        output_ = qEnvironmentVariable("CLIPFORGE_TEST_OUTPUT");
        QVERIFY2(QFileInfo::exists(source_), "CLIPFORGE_TEST_VIDEO must point to an actual captured clip");
    }
    void probeAndDecode() {
        QProcess p;
        p.start(tools_ + "/ffprobe.exe",
                {"-v", "error", "-show_streams", "-show_format", "-of", "json", source_});
        QVERIFY(p.waitForFinished(10000));
        QCOMPARE(p.exitCode(), 0);
        auto doc = QJsonDocument::fromJson(p.readAllStandardOutput()).object();
        bool video = false, audio = false;
        for (auto v : doc["streams"].toArray()) {
            auto s = v.toObject();
            if (s["codec_type"] == "video") {
                video = true;
                QCOMPARE(s["width"].toInt(), 1280);
                QCOMPARE(s["height"].toInt(), 720);
                QCOMPARE(s["r_frame_rate"].toString(), QString("30/1"));
            }
            if (s["codec_type"] == "audio")
                audio = true;
        }
        QVERIFY(video);
        QVERIFY(audio);
        QVERIFY(doc["format"].toObject()["duration"].toString().toDouble() >= 12);
        p.start(tools_ + "/ffmpeg.exe", {"-v", "error", "-nostdin", "-i", source_, "-f", "null", "-"});
        QVERIFY(p.waitForFinished(30000));
        QCOMPARE(p.exitCode(), 0);
        QVERIFY2(p.readAllStandardError().isEmpty(), "Decode produced errors");
    }
    void nativePlayback() {
        QMediaPlayer player;
        QVideoSink sink;
        cf::PlaybackAudio audio;
        audio.setVolume(0);
        audio.setMediaPlayer(&player);
        player.setVideoOutput(&sink);
        QSignalSpy frames(&sink, &QVideoSink::videoFrameChanged);
        player.setSource(QUrl::fromLocalFile(source_));
        player.play();
        QTRY_VERIFY_WITH_TIMEOUT(frames.count() > 3, 12000);
        QVERIFY(sink.videoFrame().isValid());
        QTRY_VERIFY_WITH_TIMEOUT(player.position() > 0, 5000);
        QTRY_VERIFY_WITH_TIMEOUT(audio.processedUSecs() > 100000, 5000);
        QVERIFY2(audio.error().isEmpty(), qPrintable(audio.error()));
        QTest::qWait(1000);
        QVERIFY(player.position() < 4000);
        const auto beforeFastPlayback = player.position();
        const auto audioBeforeFastPlayback = audio.processedUSecs();
        player.setPlaybackRate(2.0);
        player.setPlaybackRate(4.0);
        QTRY_VERIFY_WITH_TIMEOUT(player.position() >= beforeFastPlayback + 1200, 3000);
        QTRY_VERIFY_WITH_TIMEOUT(audio.processedUSecs() > audioBeforeFastPlayback + 100000, 3000);
        QVERIFY2(audio.error().isEmpty(), qPrintable(audio.error()));
        player.setPlaybackRate(1.0);
        audio.seek(3000);
        QTRY_VERIFY_WITH_TIMEOUT(player.position() >= 3000, 5000);
        player.stop();
    }
    void audioDecode() {
        QAudioDecoder decoder;
        QSignalSpy buffers(&decoder, &QAudioDecoder::bufferReady);
        decoder.setSource(QUrl::fromLocalFile(source_));
        decoder.start();
        QTRY_VERIFY_WITH_TIMEOUT(buffers.count() > 0, 10000);
        auto buffer = decoder.read();
        QVERIFY(buffer.isValid());
        QCOMPARE(buffer.format().sampleRate(), 48000);
        QCOMPARE(buffer.format().channelCount(), 2);
        decoder.stop();
    }
    void globalHotkey() {
        cf::GlobalHotkeys hotkeys;
        QVERIFY(hotkeys.configure(VK_F22).isEmpty());
        QSignalSpy saved(&hotkeys, &cf::GlobalHotkeys::save);
        INPUT keys[2]{};
        keys[0].type = INPUT_KEYBOARD;
        keys[0].ki.wVk = VK_F22;
        keys[1] = keys[0];
        keys[1].ki.dwFlags = KEYEVENTF_KEYUP;
        QCOMPARE(SendInput(2, keys, sizeof(INPUT)), UINT(2));
        QTRY_COMPARE_WITH_TIMEOUT(saved.count(), 1, 3000);
        QVERIFY(RegisterHotKey(nullptr, 0xCF77, 0, VK_F21));
        QVERIFY(!hotkeys.configure(VK_F21).isEmpty());
        UnregisterHotKey(nullptr, 0xCF77);
    }
    void preciseExport() {
        QVERIFY(!output_.isEmpty());
        QFile::remove(output_);
        cf::EditProject project;
        project.source = source_;
        project.segments = {{1, 2, 0.5}, {4, 6, 2}};
        cf::MediaJobs jobs(tools_, QFileInfo(output_).absolutePath() + "/thumb");
        QSignalSpy done(&jobs, &cf::MediaJobs::exported), errors(&jobs, &cf::MediaJobs::failure);
        QVERIFY(jobs.exportVideo(project, output_, {.height = 720, .fps = 30}));
        QTRY_VERIFY_WITH_TIMEOUT(done.count() + errors.count() > 0, 45000);
        if (errors.count())
            qWarning() << errors.first();
        QCOMPARE(errors.count(), 0);
        QCOMPARE(done.count(), 1);
        QProcess p;
        p.start(tools_ + "/ffprobe.exe",
                {"-v", "error", "-show_entries", "format=duration", "-of", "default=nw=1:nk=1", output_});
        QVERIFY(p.waitForFinished(10000));
        double d = p.readAllStandardOutput().trimmed().toDouble();
        QVERIFY2(std::abs(d - 3) < 0.2, qPrintable(QString::number(d)));
        QVERIFY(!jobs.exportVideo(project, output_, {.height = 720, .fps = 30}));
    }
};
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    MediaTests test;
    return QTest::qExec(&test, argc, argv);
}
#include "MediaTests.moc"
