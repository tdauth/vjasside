#ifndef PJASS_H
#define PJASS_H

#include <QProcess>
#include <QObject>
#include <QString>

#include "vjassparseerror.h"

/**
 * @brief The PJass class supports executing the popular JASS parser <a href="https://github.com/lep/pjass">pjass</a>.
 */
// https://www.hiveworkshop.com/threads/pjass-updates.258738/
class PJass : public QObject
{
    Q_OBJECT

public:
    PJass(QObject *parent = nullptr);

    int run(const QString &filePath, const QString &commonj, const QString &commonai, const QString &blizzardj, const QString &code);
    int run(const QString &commonj, const QString &commonai, const QString &blizzardj, const QString &code);
    int run(const QString &code);

    const QString& getStandardOutput() const;
    const QString& getStandardError() const;

    static QList<VJassParseError> outputToParseErrors(const QString &output);

private slots:
    void readStandardOutput();
    void readStandardError();
    void errorOcurred(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess process;
    QString standardOutput;
    QString standardError;
};

#endif // PJASS_H
