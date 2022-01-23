#ifndef TESTSCANNER_H
#define TESTSCANNER_H

#include <QTest>

class TestScanner : public QObject
{
    Q_OBJECT

    private slots:
        void canScanFunction();
        void canScanNativesFromCommonJ();
        void canScanCommonJ();
        void canScanCommonAI();
        void canScanCommonBlizzardJ();
};

#endif // TESTSCANNER_H
