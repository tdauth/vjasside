#include <QtTest>

#include "../../app/vjassscanner.h"
#include "testscanner.h"

void TestScanner::canScanFunction()
{
    VJassScanner scanner;

    QList<VJassToken> tokens = scanner.scan("function bla takes nothing returns nothing\nendfunction");

    QCOMPARE(tokens.size(), 8);
}

void TestScanner::canScanNativesFromCommonJ() {
    /*
     native GroupEnumUnitsInRangeOfLocCounted    takes group whichGroup, location whichLocation, real radius, boolexpr filter, integer countLimit returns nothing
     native GroupEnumUnitsSelected               takes group whichGroup, player whichPlayer, boolexpr filter returns nothing
     */
    VJassScanner scanner;

    QList<VJassToken> tokens = scanner.scan(
        QString("native GroupEnumUnitsInRangeOfLocCounted    takes group whichGroup, location whichLocation, real radius, boolexpr filter, integer countLimit returns nothing\n")
        + "native GroupEnumUnitsSelected               takes group whichGroup, player whichPlayer, boolexpr filter returns nothing");

    QCOMPARE(tokens.size(), 33);

    int index = -1;

    for (int i = 1; i < tokens.size() && index == -1; ++i) {
        if (tokens.at(i).getType() == VJassToken::NativeKeyword) {
            index = i;
        }
    }

    QCOMPARE(index, 20);
    QCOMPARE(tokens.at(index).getColumn(), 0);
    QCOMPARE(tokens.at(index).getLine(), 1);
    QCOMPARE(tokens.at(index).getValue(), "native");
    QCOMPARE(tokens.at(index).getValueLength(), 6);
}

void TestScanner::canScanCommonJ() {
    QFile f("wc3reforged/common.j");

    QVERIFY(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString input = in.readAll();
    QCOMPARE(input.size(), 355533);

    VJassScanner scanner;

    QList<VJassToken> tokens;

    QBENCHMARK {
        tokens = scanner.scan(input, false);
    }

    QCOMPARE(tokens.size(), 57321);
    QCOMPARE(input.size(), 355533);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 35585);
    QCOMPARE(input.size(), 355533);
}

void TestScanner::canScanCommonAI() {
    QFile f("wc3reforged/common.ai");

    QVERIFY(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString input = in.readAll();
    QCOMPARE(input.size(), 95876);

    VJassScanner scanner;

    QList<VJassToken> tokens;

    QBENCHMARK {
        tokens = scanner.scan(input, false);
    }

    QCOMPARE(tokens.size(), 8461);
    QCOMPARE(input.size(), 95876);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 5077);
    QCOMPARE(input.size(), 95876);
}

void TestScanner::canScanCommonBlizzardJ() {
    QFile f("wc3reforged/Blizzard.j");

    QVERIFY(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString input = in.readAll();
    QCOMPARE(input.size(), 471054);

    VJassScanner scanner;

    QList<VJassToken> tokens;

    QBENCHMARK {
        tokens = scanner.scan(input, false);
    }

    QCOMPARE(tokens.size(), 5733);
    QCOMPARE(input.size(), 471054);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 3354);
    QCOMPARE(input.size(), 471054);
}

QTEST_MAIN(TestScanner)
