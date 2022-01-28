#include <QtTest>

#include "../../app/highlightinfo.h"
#include "../../app/vjassscanner.h"
#include "../../app/vjassparser.h"
#include "testhighlightinfo.h"

void TestHighlightInfo::canHoldTokens() {
    const QString text = "function test";
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(text, false);

    HighLightInfo highLightInfo(text, tokens, nullptr);

    QCOMPARE(highLightInfo.getFormattedLocations().size(), 1);
    QVERIFY(highLightInfo.getFormattedLocations().contains(HighLightInfo::Location(0, 0)));
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isBold, true);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isItalic, false);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].length, 8);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].applyForegroundColor, true);
}

void TestHighlightInfo::canHoldTokensFromCommonJ() {
    /*
     native GroupEnumUnitsInRangeOfLocCounted    takes group whichGroup, location whichLocation, real radius, boolexpr filter, integer countLimit returns nothing
     native GroupEnumUnitsSelected               takes group whichGroup, player whichPlayer, boolexpr filter returns nothing
     */
    const QString text = QString("native GroupEnumUnitsInRangeOfLocCounted    takes group whichGroup, location whichLocation, real radius, boolexpr filter, integer countLimit returns nothing\n")
            + "native GroupEnumUnitsSelected               takes group whichGroup, player whichPlayer, boolexpr filter returns nothing";
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(text, false);

    HighLightInfo highLightInfo(text, tokens, nullptr);

    // native GroupEnumUnitsInRangeOfLocCounted    takes group location real radius boolexpr integer returns nothing
    // native GroupEnumUnitsSelected               takes group player boolexpr filter returns nothing
    QCOMPARE(highLightInfo.getFormattedLocations().size(), 18);
    QVERIFY(highLightInfo.getFormattedLocations().contains(HighLightInfo::Location(0, 0)));
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isBold, true);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isItalic, false);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].length, 6); // native
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].applyForegroundColor, true);

    // second native
    QVERIFY(highLightInfo.getFormattedLocations().contains(HighLightInfo::Location(1, 0)));
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isBold, true);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isItalic, false);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].length, 6); // native
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].applyForegroundColor, true);
}


void TestHighlightInfo::canHoldTokensFromBlizzardJ() {
    const QString text = QString("bj_PI");
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(text, false);

    HighLightInfo highLightInfo(text, tokens, nullptr);

    QCOMPARE(highLightInfo.getFormattedLocations().size(), 1);
    QVERIFY(highLightInfo.getFormattedLocations().contains(HighLightInfo::Location(0, 0)));
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isBold, false);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].isItalic, true);
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].length, 5); // bj_PI
    QCOMPARE(highLightInfo.getFormattedLocations()[HighLightInfo::Location(0, 0)].applyForegroundColor, true);
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
