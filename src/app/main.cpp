#include "Controller.h"
#include "playback/PlaybackAudio.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QLockFile>
#include <QMutex>
#include <QMutexLocker>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>
#include <QVideoSink>
#include <QVideoFrame>
#include <cstdarg>
#include <objbase.h>
#include <windows.h>
namespace {
QFile logfile;
QMutex logMutex;
void writeLog(QString s) {
    QMutexLocker lock(&logMutex);
    if (logfile.isOpen()) {
        logfile.write((QDateTime::currentDateTime().toString(Qt::ISODateWithMs) + " " + s + "\n").toUtf8());
        logfile.flush();
    }
}
void obsLog(int level, const char *format, va_list args, void *) {
    if (level > LOG_INFO)
        return;
    char text[8192];
    vsnprintf(text, sizeof(text), format, args);
    writeLog(QString::fromUtf8(text));
}
} // namespace
int main(int argc, char **argv) {
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    QApplication app(argc, argv);
    app.setFont(QFont("Segoe UI", 10));
    app.setApplicationName("ClipForge");
    app.setOrganizationName("ClipForge");
    app.setApplicationVersion("0.1.0");
    app.setQuitOnLastWindowClosed(false);
    QQuickStyle::setStyle("Basic");
    qmlRegisterType<cf::PlaybackAudio>("ClipForge.Playback", 1, 0, "PlaybackAudio");
    const auto args = app.arguments();
    bool test = args.contains("--self-test");
    auto argument = [&](QString key, QString fallback) {
        int i = args.indexOf(key);
        return i >= 0 && i + 1 < args.size() ? args[i + 1] : fallback;
    };
    QString data =
        argument("--data-dir", QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    QDir().mkpath(data);
    QLockFile lock(data + "/instance.lock");
    if (!lock.tryLock(0))
        return 2;
    logfile.setFileName(data + "/clipforge.log");
    if (logfile.size() > 8 * 1024 * 1024) {
        QFile::remove(data + "/clipforge.previous.log");
        logfile.rename(data + "/clipforge.previous.log");
        logfile.setFileName(data + "/clipforge.log");
    }
    logfile.open(QIODevice::WriteOnly | QIODevice::Append);
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext &, const QString &s) { writeLog(s); });
    base_set_log_handler(obsLog, nullptr);
    auto dir = QCoreApplication::applicationDirPath();
    QString runtime = argument("--obs-runtime", dir + "/obs");
    SetDllDirectoryW(reinterpret_cast<LPCWSTR>(QDir::toNativeSeparators(runtime + "/bin/64bit").utf16()));
    // OBS mux and GPU capability subprocesses resolve their helpers from the OBS binary directory.
    QDir::setCurrent(runtime + "/bin/64bit");
    int result = 0;
    {
        QProcess fixture;
        if (args.contains("--window-test")) {
            fixture.start(dir + "/ClipForgeFixture.exe");
            fixture.waitForStarted(5000);
            QThread::msleep(300);
        }
        cf::Controller controller(runtime, data, dir + "/tools", test);
        QQmlApplicationEngine engine;
        engine.rootContext()->setContextProperty("appController", &controller);
        engine.rootContext()->setContextProperty("clipLibrary", controller.library());
        qInfo("Loading QML");
        engine.load(QUrl("qrc:/ui/App.qml"));
        qInfo("QML loaded");
        if (engine.rootObjects().isEmpty())
            return 3;
        auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QObject::connect(&controller, &cf::Controller::showWindow, window, [window] {
            window->showNormal();
            window->raise();
            window->requestActivate();
        });
        if(args.contains("--offscreen-window")){window->setFlags(window->flags()|Qt::Tool|Qt::WindowDoesNotAcceptFocus);window->setPosition(-8000,-8000);}
        if (test || args.contains("--headless"))
            window->hide();
        else if (!controller.config()["startMinimized"].toBool())
            window->show();
        if (args.contains("--screenshot")) {
            QTimer::singleShot(2200, &app, [window, data] { window->grabWindow().save(data + "/ui.png"); });
            QTimer::singleShot(3000, &app, &QApplication::quit);
        }
        if (args.contains("--ui-test")) {
            for (int page = 0; page < 6; ++page) {
                QTimer::singleShot(1200 + page * 1400, &app, [window, page, &controller] {
                    window->setProperty("page", page);
                    if (page == 3 && controller.library()->count() > 0) {
                        controller.selectClip(0);
                        if (auto audio = window->findChild<QObject *>("playbackAudio"))
                            audio->setProperty("volume", 0);
                        if (auto player = window->findChild<QObject *>("clipPlayer"))
                            QMetaObject::invokeMethod(player, "play");
                    }
                    if (page == 5) {
                        auto field = window->findChild<QObject *>("replaySecondsField");
                        if (field)
                            field->setProperty("text", "90");
                    }
                });
                QTimer::singleShot(2200 + page * 1400, &app, [window, data, page] {
                    window->grabWindow().save(data + QString("/page-%1.png").arg(page));
                });
            }
            QTimer::singleShot(10100, &app, [window, &app] {
                auto field = window->findChild<QObject *>("replaySecondsField");
                if (!field || field->property("text").toString() != "90") {
                    qCritical("UI validation failed: settings reset during metrics refresh");
                    app.exit(5);
                } else
                    qInfo("UI validation passed: edited settings survive metrics refresh");
            });
            QTimer::singleShot(11000, &app, &QApplication::quit);
        }
        if(args.contains("--editor-test")){
            QTimer::singleShot(1500,&app,[window,&controller]{if(controller.library()->count()>0)controller.selectClip(controller.library()->count()>1?1:0);window->setProperty("page",3);});
            QTimer::singleShot(2500,&app,[window]{if(auto audio=window->findChild<QObject*>("playbackAudio"))audio->setProperty("volume",0);if(auto player=window->findChild<QObject*>("clipPlayer"))QMetaObject::invokeMethod(player,"play");});
            QTimer::singleShot(9500,&app,[window,data,&app]{window->grabWindow().save(data+"/editor-playing.png");auto player=qobject_cast<QMediaPlayer*>(window->findChild<QObject*>("clipPlayer"));if(!player||player->position()<1000||!player->videoSink()||!player->videoSink()->videoFrame().isValid()){qCritical("Editor playback validation failed");app.exit(6);}else qInfo("Editor playback validation passed: actual video frame and advancing position");});
            QTimer::singleShot(11000,&app,&QApplication::quit);
        }
        result = app.exec();
        if (fixture.state() != QProcess::NotRunning) {
            fixture.kill();
            fixture.waitForFinished(3000);
        }
    }
    CoUninitialize();
    return result;
}
