#include <QtTest>

#include "vjassscanner.h"
#include "testscanner.h"

void TestScanner::canScanFunction()
{
    VJassScanner scanner;

    QList<VJassToken> tokens = scanner.scan("function bla takes nothing returns nothing\nendfunction");

    QCOMPARE(tokens.size(), 3);
}

//QTEST_MAIN(TestScanner)
//#include "testscanner.moc"
