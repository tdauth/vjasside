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

void TestScanner::canScanRandomCharacters() {
    const QString RANDOM_CHARACTER_TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXZY0123456789!\"§$%&()=?\\ßäöü+*-/#_':;,<>|°^{}[]\t ";

    QString randomString = "";

    for (int i = 0; i < 1000; ++i) {
        int index = QRandomGenerator::global()->bounded(RANDOM_CHARACTER_TABLE.length() - 1);
        randomString += RANDOM_CHARACTER_TABLE.mid(index, 1);
    }

    VJassScanner scanner;

    scanner.scan(randomString);
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

    QCOMPARE(tokens.size(), 57314);
    QCOMPARE(input.size(), 355533);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 35578);
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

    QCOMPARE(tokens.size(), 20870);
    QCOMPARE(input.size(), 95876);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 13362);
    QCOMPARE(input.size(), 95876);
}

void TestScanner::canScanBlizzardJ() {
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

    QCOMPARE(tokens.size(), 84288);
    QCOMPARE(input.size(), 471054);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 56039);
    QCOMPARE(input.size(), 471054);
}

QTEST_MAIN(TestScanner)
