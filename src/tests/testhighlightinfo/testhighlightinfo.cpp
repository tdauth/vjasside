#include <QtTest>

#include "../../app/highlightinfo.h"
#include "../../app/vjassscanner.h"
#include "../../app/vjassparser.h"
#include "testhighlightinfo.h"

void TestHighlightInfo::canHoldTokens() {
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan("function test", false);

    HighLightInfo codeElementHolder(tokens, nullptr);

    QCOMPARE(codeElementHolder.getFormattedLocations().size(), 8);
    QVERIFY(codeElementHolder.getFormattedLocations().contains(HighLightInfo::Location(0, 0)));
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].isBold, true);
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].isItalic, false);
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].length, 7);
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].applyForegroundColor, false);
}

void TestHighlightInfo::canHoldTokensFromCommonJ() {
    /*
     native GroupEnumUnitsInRangeOfLocCounted    takes group whichGroup, location whichLocation, real radius, boolexpr filter, integer countLimit returns nothing
     native GroupEnumUnitsSelected               takes group whichGroup, player whichPlayer, boolexpr filter returns nothing
     */
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(
        QString("native GroupEnumUnitsInRangeOfLocCounted    takes group whichGroup, location whichLocation, real radius, boolexpr filter, integer countLimit returns nothing\n")
        + "native GroupEnumUnitsSelected               takes group whichGroup, player whichPlayer, boolexpr filter returns nothing"
    , false);

    HighLightInfo codeElementHolder(tokens, nullptr);

    // native GroupEnumUnitsInRangeOfLocCounted    takes group location real radius boolexpr integer returns nothing
    // native GroupEnumUnitsSelected               takes group player boolexpr filter returns nothing
    QCOMPARE(codeElementHolder.getFormattedLocations().size(), 20);
    QVERIFY(codeElementHolder.getFormattedLocations().contains(HighLightInfo::Location(0, 0)));
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].isBold, true);
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].isItalic, false);
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].length, 6); // native
    QCOMPARE(codeElementHolder.getFormattedLocations()[HighLightInfo::Location(0, 0)].applyForegroundColor, true);
}

/*
void TestHighlightInfo::canOrderCodeElementsFromCommonJ() {
    QFile f("wc3reforged/common.j");

    QVERIFY(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString input = in.readAll();
    QCOMPARE(input.size(), 355533);

    VJassScanner scanner;

    QList<VJassToken> tokens = scanner.scan(input, false);

    QCOMPARE(tokens.size(), 57321);
    QCOMPARE(input.size(), 355533);

    VJassParser parser;
    VJassAst *ast = parser.parse(tokens);

    QBENCHMARK {
        VJassCodeElementHolder codeElementHolder(tokens, ast);
    }

    VJassCodeElementHolder codeElementHolder(tokens, ast);
    QCOMPARE(codeElementHolder.getSize(), 492838);

    //delete ast;
    //ast = nullptr;
}
*/

QTEST_MAIN(TestHighlightInfo)
