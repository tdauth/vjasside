#include <QtTest>

#include "../app/vjassscanner.h"
#include "testscanner.h"

void TestScanner::canScanFunction()
{
    VJassScanner scanner;

    QList<VJassToken> tokens = scanner.scan("function bla takes nothing returns nothing\nendfunction");

    QCOMPARE(tokens.size(), 8);
}

void TestScanner::canScanCommonJ() {
    QFile f("wc3reforged/common.j");

    QVERIFY(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString input = in.readAll();
    QCOMPARE(input.size(), 355533);

    VJassScanner scanner;

    QList<VJassToken> tokens = scanner.scan(input, false);

    QCOMPARE(tokens.size(), 68582);
    QCOMPARE(input.size(), 355533);

    tokens = scanner.scan(input, true);

    QCOMPARE(tokens.size(), 46846);
    QCOMPARE(input.size(), 355533);
}

QTEST_MAIN(TestScanner)
//#include "testscanner.moc"
