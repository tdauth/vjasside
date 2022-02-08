#ifndef JASSHELPER_H
#define JASSHELPER_H

#include <QObject>
#include <QProcess>

#include "vjassparseerror.h"

class JassHelper : public QObject
{
    Q_OBJECT

public:
    JassHelper(const QString &filePath, QObject *parent = nullptr);
    JassHelper(QObject *parent = nullptr);

    int run(const QString &commonj, const QString &commonai, const QString &blizzardj, const QString &code);
    int run(const QString &code);
    int runVersion();

    const QString& getStandardOutput() const;
    const QString& getStandardError() const;
    const QString& getVersion() const;

    static QList<VJassParseError> outputToParseErrors(const QString &output);

private slots:
    void readStandardOutput();
    void readStandardError();
    void errorOcurred(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QString filePath;
    QProcess process;
    QString standardOutput;
    QString standardError;
    QString version;
};

#endif // JASSHELPER_H
