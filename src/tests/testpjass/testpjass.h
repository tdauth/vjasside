#ifndef TESTPARSER_H
#define TESTPARSER_H

#include <QTest>

class TestPJass : public QObject
{
    Q_OBJECT

    private slots:
        void canRunPJass();
};

#endif // TESTPARSER_H
