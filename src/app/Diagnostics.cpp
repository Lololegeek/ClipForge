#include "Controller.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QScreen>
#include <QGuiApplication>
#include <QSysInfo>
#include <thread>
#include <windows.h>
#include <dbghelp.h>

namespace cf {
namespace {
wchar_t g_crashDir[MAX_PATH] = {};

LONG WINAPI crashFilter(EXCEPTION_POINTERS *info) {
    static LONG busy = 0;
    if (InterlockedExchange(&busy, 1))
        return EXCEPTION_CONTINUE_SEARCH;
    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t path[MAX_PATH + 64];
    swprintf(path, MAX_PATH + 64, L"%s\crash-%04d%02d%02d-%02d%02d%02d.dmp", g_crashDir, t.wYear, t.wMonth, t.wDay,
             t.wHour, t.wMinute, t.wSecond);
    HANDLE f = CreateFileW(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (f != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mei{GetCurrentThreadId(), info, FALSE};
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), f, MiniDumpWithIndirectlyReferencedMemory,
                          &mei, nullptr, nullptr);
        CloseHandle(f);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// Removes what is personal: webhook links, tokens, and the Windows user name inside paths.
QString scrub(QString text) {
    static const QRegularExpression hook(R"(https?://[^\s"']*discord(app)?\.com/api/webhooks/[^\s"']*)",
                                         QRegularExpression::CaseInsensitiveOption);
    text.replace(hook, "<webhook masqué>");
    const QString home = QDir::homePath();
    if (!home.isEmpty()) {
        text.replace(home, "%USERPROFILE%", Qt::CaseInsensitive);
        text.replace(QDir::toNativeSeparators(home).replace("\\", "\\\\"), "%USERPROFILE%", Qt::CaseInsensitive);
        text.replace(QDir::toNativeSeparators(home), "%USERPROFILE%", Qt::CaseInsensitive);
    }
    return text;
}
void copyScrubbed(const QString &from, const QString &to) {
    QFile in(from);
    if (!in.open(QIODevice::ReadOnly))
        return;
    QFile out(to);
    if (out.open(QIODevice::WriteOnly))
        out.write(scrub(QString::fromUtf8(in.readAll())).toUtf8());
}
QString run(const QString &program, const QStringList &args, int ms = 20000) {
    QProcess p;
    p.start(program, args);
    if (!p.waitForFinished(ms)) {
        p.kill();
        return {};
    }
    return QString::fromLocal8Bit(p.readAllStandardOutput());
}
} // namespace

void installCrashHandler(const QString &data) {
    const QString dir = QDir::toNativeSeparators(data + "/crashes");
    QDir().mkpath(dir);
    // Keep only the five most recent dumps.
    auto old = QDir(dir).entryInfoList({"*.dmp"}, QDir::Files, QDir::Time);
    for (int i = 5; i < old.size(); ++i)
        QFile::remove(old[i].absoluteFilePath());
    dir.toWCharArray(g_crashDir);
    SetUnhandledExceptionFilter(crashFilter);
}

void Controller::createDiagnosticReport() {
    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString zip = QDir::cleanPath(data_ + "/ClipForge-diagnostic-" + stamp + ".zip");
    const QString data = data_;
    const QString version = QString(CF_VERSION_LABEL);
    notify("Création du rapport de diagnostic…");
    std::thread([this, zip, data, version, stamp] {
        const QString stage = data + "/diag-" + stamp;
        QDir().mkpath(stage);
        copyScrubbed(data + "/clipforge.log", stage + "/clipforge.log");
        copyScrubbed(data + "/clipforge.previous.log", stage + "/clipforge.previous.log");
        QDir().mkpath(stage + "/crashes");
        for (const auto &f : QDir(data + "/crashes").entryInfoList({"*.dmp"}, QDir::Files, QDir::Time).mid(0, 3))
            QFile::copy(f.absoluteFilePath(), stage + "/crashes/" + f.fileName());
        // Settings without secrets.
        QFile sf(data + "/settings.json");
        if (sf.open(QIODevice::ReadOnly)) {
            auto doc = QJsonDocument::fromJson(sf.readAll());
            QJsonObject o = doc.object();
            static const QRegularExpression secret("webhook|token|secret|password", QRegularExpression::CaseInsensitiveOption);
            for (const auto &k : o.keys())
                if (secret.match(k).hasMatch() && !o.value(k).toString().isEmpty())
                    o[k] = "<masqué>";
            QFile out(stage + "/settings.json");
            if (out.open(QIODevice::WriteOnly))
                out.write(scrub(QString::fromUtf8(QJsonDocument(o).toJson())).toUtf8());
        }
        // System and graphics card.
        QString info = "ClipForge " + version + "\n";
        info += "Windows : " + QSysInfo::prettyProductName() + " (" + QSysInfo::kernelVersion() + ")\n";
        info += "Processeur : " + QSysInfo::currentCpuArchitecture() + "\n\n";
        info += "Cartes graphiques :\n" +
                run("powershell", {"-NoProfile", "-Command",
                                   "Get-CimInstance Win32_VideoController | ForEach-Object { $_.Name + ' | pilote ' + $_.DriverVersion + ' | ' + $_.CurrentHorizontalResolution + 'x' + $_.CurrentVerticalResolution }"});
        QFile out(stage + "/systeme.txt");
        if (out.open(QIODevice::WriteOnly))
            out.write(info.toUtf8());
        QFile::remove(zip);
        run("tar", {"-a", "-c", "-f", QDir::toNativeSeparators(zip), "-C", QDir::toNativeSeparators(stage), "."}, 60000);
        QDir(stage).removeRecursively();
        const bool ok = QFile::exists(zip);
        QMetaObject::invokeMethod(this, [this, zip, ok] {
            if (ok) {
                notify("Rapport créé : aucun mot de passe ni lien Discord n'y figure.");
                reveal(zip);
            } else {
                notify("Impossible de créer le rapport.", true);
            }
        });
    }).detach();
}
} // namespace cf
