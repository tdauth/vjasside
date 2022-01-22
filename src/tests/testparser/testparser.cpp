#include <QtTest>

#include "../../app/vjassscanner.h"
#include "../../app/vjassparser.h"
#include "testparser.h"

void TestParser::canParseCommonJ() {
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

    QCOMPARE(tokens.size(), 57546);
    QCOMPARE(input.size(), 355533);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 35810);
    QCOMPARE(input.size(), 355533);

    VJassParser parser;
    VJassAst *ast = nullptr;

    QBENCHMARK {
        ast = parser.parse(tokens);
    }

    QCOMPARE(ast->getChildren().size(), 1668);
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}

QTEST_MAIN(TestParser)
