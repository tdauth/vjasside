#ifndef TESTSCANNER_H
#define TESTSCANNER_H

#include <QTest>

class TestScanner : public QObject
{
    Q_OBJECT

    private slots:
        void canScanFunction();
};

#endif // TESTSCANNER_H
