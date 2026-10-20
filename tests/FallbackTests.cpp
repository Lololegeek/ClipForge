#include "capture/ObsEngine.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>
#include <memory>
#include <windows.h>

// Optional integration test: requires the packaged OBS runtime and a Windows GPU.
class FallbackTests : public QObject {
    Q_OBJECT
    QTemporaryDir data_;
    std::unique_ptr<cf::ObsEngine> engine_;
    QProcess fixture_;
    QString package_;

    cf::Settings settings() const {
        auto s = cf::Settings::defaults();
        s.clips = data_.filePath("clips");
        s.width = 1280;
        s.height = 720;
        s.fps = 30;
        s.bitrate = 3000;
        s.mode = "window";
        s.target = "ClipForge Capture Fixture:ClipForgeFixture:ClipForgeFixture.exe";
        s.audio = "disabled";
        return s;
    }

  private slots:
    void initTestCase() {
        QVERIFY(data_.isValid());
        package_ = qEnvironmentVariable("CLIPFORGE_TEST_PACKAGE",
                                       QCoreApplication::applicationDirPath() + "/../dist/ClipForge");
        package_ = QDir(package_).absolutePath();
        const auto binaries = QDir::toNativeSeparators(package_ + "/obs/bin/64bit");
        SetDllDirectoryW(reinterpret_cast<LPCWSTR>(binaries.utf16()));
        QVERIFY(QDir::setCurrent(binaries));
        fixture_.start(package_ + "/ClipForgeFixture.exe");
        QVERIFY(fixture_.waitForStarted());
        QTest::qWait(400);
    }
    void init() {
        engine_ = std::make_unique<cf::ObsEngine>(package_ + "/obs", data_.path());
        QSignalSpy ready(engine_.get(), &cf::ObsEngine::ready);
        engine_->initialize();
        QCOMPARE(ready.count(), 1);
    }
    void cleanup() {
        engine_.reset();
    }
    void hardwareFaultFallsBackAndSaves() {
        auto s = settings();
        for (const auto id : {"h264_texture_amf", "obs_nvenc_h264_tex", "obs_qsv11_v2"}) {
            if (obs_get_encoder_codec(id)) {
                s.encoder = id;
                break;
            }
        }
        if (s.encoder == "auto")
            QSKIP("No hardware H.264 encoder on this host");
        QSignalSpy notices(engine_.get(), &cf::ObsEngine::notice);
        QSignalSpy failures(engine_.get(), &cf::ObsEngine::failure);
        QSignalSpy saved(engine_.get(), &cf::ObsEngine::saved);
        engine_->start(s);
        auto output = obs_get_output_by_name("ClipForge replay");
        QVERIFY(output);
        QVERIFY(obs_output_active(output));
        const QString initial = obs_encoder_get_id(obs_output_get_video_encoder(output));
        if (initial == "obs_x264") {
            obs_output_release(output);
            QSKIP("Hardware encoder could not start on this host");
        }
        QTest::qWait(1500);
        // Inject the same OBS stop signal emitted by an asynchronous encoder failure.
        calldata_t event{};
        calldata_set_int(&event, "code", OBS_OUTPUT_ENCODE_ERROR);
        signal_handler_signal(obs_output_get_signal_handler(output), "stop", &event);
        calldata_free(&event);
        obs_output_release(output);
        QTRY_COMPARE_WITH_TIMEOUT(notices.count(), 1, 10000);
        output = obs_get_output_by_name("ClipForge replay");
        QVERIFY(output);
        const QString replacement = obs_encoder_get_id(obs_output_get_video_encoder(output));
        const bool active = obs_output_active(output);
        obs_output_release(output);
        QCOMPARE(replacement, QString("obs_x264"));
        QVERIFY(active);
        QTest::qWait(3500);
        engine_->saveReplay();
        QTRY_COMPARE_WITH_TIMEOUT(saved.count(), 1, 15000);
        QCOMPARE(failures.count(), 0);
        const auto path = saved.first().first().toString();
        QProcess decode;
        decode.start(package_ + "/tools/ffmpeg.exe",
                     {"-v", "error", "-nostdin", "-i", path, "-f", "null", "-"});
        QVERIFY(decode.waitForFinished(20000));
        QCOMPARE(decode.exitCode(), 0);
        QVERIFY(decode.readAllStandardError().isEmpty());
    }
    void unavailableCodecFallsBackToH264() {
        for (const auto id : {"av1_texture_amf", "obs_nvenc_av1_tex", "obs_qsv11_av1"})
            if (obs_get_encoder_codec(id))
                QSKIP("Host has an AV1 encoder; no unavailable-codec scenario");
        auto s = settings();
        s.codec = "av1";
        QSignalSpy notices(engine_.get(), &cf::ObsEngine::notice);
        engine_->start(s);
        auto output = obs_get_output_by_name("ClipForge replay");
        QVERIFY(output);
        const QString encoder = obs_encoder_get_id(obs_output_get_video_encoder(output));
        const bool active = obs_output_active(output);
        obs_output_release(output);
        QCOMPARE(encoder, QString("obs_x264"));
        QVERIFY(active);
        QCOMPARE(notices.count(), 1);
    }
    void cleanupTestCase() {
        fixture_.kill();
        fixture_.waitForFinished();
    }
};
QTEST_GUILESS_MAIN(FallbackTests)
#include "FallbackTests.moc"
