#include <QtCore>

#include "jasshelper.h"

JassHelper::JassHelper(const QString &filePath, QObject *parent) : QObject(parent), filePath(filePath), process(this) {
    connect(&process, &QProcess::readyReadStandardOutput, this, &JassHelper::readStandardOutput);
    connect(&process, &QProcess::readyReadStandardError, this, &JassHelper::readStandardError);
    connect(&process, &QProcess::errorOccurred, this, &JassHelper::errorOcurred);
    connect(&process, SIGNAL(finished(int , QProcess::ExitStatus)), this,  SLOT(JassHelper::finished(int , QProcess::ExitStatus)));
}

JassHelper::JassHelper(QObject *parent) {
#ifdef Q_OS_WIN32
    JassHelper("jasshelper/jasshelper.exe", parent);
#else
    JassHelper("jasshelper/jasshelper.exes", parent); // TODO linux binary or clijasshelper.exe
#endif
}

int JassHelper::run(const QString &commonj, const QString &commonai, const QString &blizzardj, const QString &code) {
    QTemporaryFile file("vjasside-jasshelper-input-XXXXXX.j");

    if (file.open()) {
        qint64 written = file.write(code.toUtf8());

        qDebug() << "JassHelper file" << file.fileName() << "written" << written;

        file.close(); // syncs the content to the disk
    }

    QTemporaryFile outputFile("vjasside-jasshelper-output-XXXXXX.j");

    // pjass common.j common.ai Blizzard.j
    QStringList arguments;
    arguments << "--scriptonly" << commonj << blizzardj << file.fileName() << outputFile.fileName();

    process.start(filePath, arguments);

    process.waitForStarted();

    process.waitForReadyRead();

    process.waitForReadyRead();

    process.waitForFinished();

    qDebug() << "JassHelper exit with" << process.exitCode();

    return process.exitCode();

    // TODO Use argument - and read the code from the input.

    // make sure the process is killed in the end
    //process.kill();

    /*
    QStringList arguments;
    arguments << "-"; // - means it reads from stdin

    process.start(filePath, arguments);

    process.waitForStarted();

    qint64 written = process.write(code.toUtf8());

    qDebug() << "Written to pjass" << written;

    process.waitForBytesWritten();

    QThread::sleep(10);

    process.waitForReadyRead();

    QString result = process.readAllStandardOutput();
    result += process.readAllStandardError();

    //process.waitForReadyRead();
    //process.waitForFinished();

    qDebug() << "pjass exit with" << process.exitCode() << "and output" << result;

    // make sure the process is killed in the end
    //process.kill();

    return result;
    */
}

int JassHelper::run(const QString &code) {
    return run("wc3reforged/common.j", "wc3reforged/common.ai", "wc3reforged/Blizzard.j", code);
}

int JassHelper::runVersion() {
    QSignalBlocker signalBlocker(process);

    process.start(filePath, QStringList() << "-v");

    process.waitForStarted();

    process.waitForReadyRead();

    process.waitForReadyRead();

    process.waitForFinished();

    qDebug() << "JassHelper exit with" << process.exitCode();

    version = process.readAllStandardOutput();

    return process.exitCode();
}

const QString& JassHelper::getStandardOutput() const {
    return standardOutput;
}

const QString& JassHelper::getStandardError() const {
    return standardError;
}

const QString& JassHelper::getVersion() const {
    return version;
}

QList<VJassParseError> JassHelper::outputToParseErrors(const QString &output) {
    /*
     * Parse successful:     4198 lines: wc3reforged/common.j
     * Parse successful:     2584 lines: wc3reforged/common.ai
     * Parse successful:    10812 lines: wc3reforged/Blizzard.j
     * C:/Users/Tamino/Documents/Projekte/build-vjasside-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/src/app/vjasside-pjass-input-CktkTe.j:1: syntax error
     * C:/Users/Tamino/Documents/Projekte/build-vjasside-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/src/app/vjasside-pjass-input-CktkTe.j:1: Statement outside of function
     * C:/Users/Tamino/Documents/Projekte/build-vjasside-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/src/app/vjasside-pjass-input-CktkTe.j:1: Statement outside of function
     * C:/Users/Tamino/Documents/Projekte/build-vjasside-Desktop_Qt_6_2_2_MinGW_64_bit-Debug/src/app/vjasside-pjass-input-CktkTe.j failed with 3 errors
     */
    QStringList lines = output.split("\r\n");

    // Linux
    if (lines.size() <= 1) {
        lines = output.split("\n");
    }

    const QRegularExpression exp(":([0-9]+):(.+)");
    QList<VJassParseError> parseErrors;

    // skip the first line
    for (const QString &line : lines) {
        //qDebug() << "pjass output line" << line;
        QRegularExpressionMatch match = exp.match(line);

        if (match.hasMatch()) {
            //qDebug() << "pjass got syntax error:" << line;
            // TODO How to get the column or length?
            // start with line number 0 here
            parseErrors.push_back(VJassParseError(match.captured(1).toInt() - 1, 0, 1, match.captured(2).trimmed()));
        }
    }

    qDebug() << "Got" << parseErrors.size() << "syntax errors from pjass";

    return parseErrors;
}

void JassHelper::readStandardOutput() {
    standardOutput += process.readAllStandardOutput();

    qDebug() << "JassHelper read standard output" << getStandardOutput();
}

void JassHelper::readStandardError() {
    standardError += process.readAllStandardError();

    qDebug() << "JassHelper read standard error" << getStandardError();
}

void JassHelper::errorOcurred(QProcess::ProcessError error) {
    qDebug() << "pjass error occurred while executing" << error;
}

void JassHelper::finished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "JassHelper finished with exit code" << exitCode << "and exit status" << exitStatus;
}
