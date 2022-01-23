#include <QtTest>

#include "../../app/vjasscodeelementholder.h"
#include "../../app/vjassscanner.h"
#include "../../app/vjassparser.h"
#include "testcodeelementholder.h"

void TestCodeElementHolder::canHoldTokens() {
    VJassScanner scanner;

    QList<VJassToken> tokens = scanner.scan("function test", false);

    VJassCodeElementHolder codeElementHolder(tokens, nullptr);

    QCOMPARE(codeElementHolder.getFormattedLocations().size(), 8);
    QVERIFY(codeElementHolder.getFormattedLocations().contains(VJassCodeElementHolder::Location(0, 0)));
    QCOMPARE(codeElementHolder.getFormattedLocations()[VJassCodeElementHolder::Location(0, 0)].isBold, true);
    QCOMPARE(codeElementHolder.getFormattedLocations()[VJassCodeElementHolder::Location(0, 0)].isItalic, false);
    QCOMPARE(codeElementHolder.getFormattedLocations()[VJassCodeElementHolder::Location(0, 0)].length, 7);
    QCOMPARE(codeElementHolder.getFormattedLocations()[VJassCodeElementHolder::Location(0, 0)].applyForegroundColor, false);
}

/*
void TestCodeElementHolder::canOrderCodeElementsFromCommonJ() {
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

QTEST_MAIN(TestCodeElementHolder)
