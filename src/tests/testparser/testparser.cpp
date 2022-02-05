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

    QCOMPARE(tokens.size(), 57314);
    QCOMPARE(input.size(), 355533);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 35578);
    QCOMPARE(input.size(), 355533);

    VJassParser parser;
    VJassAst *ast = nullptr;

    QBENCHMARK {
        ast = parser.parse(tokens);
    }

    QVERIFY(ast != nullptr);
    QCOMPARE(ast->getChildren().size(), 1669);
    // no syntax errors
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}

void TestParser::canParseCommonAI() {
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

    QCOMPARE(tokens.size(), 20830);
    QCOMPARE(input.size(), 95876);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 13510);
    QCOMPARE(input.size(), 95876);

    VJassParser parser;
    VJassAst *ast = nullptr;

    QBENCHMARK {
        ast = parser.parse(tokens);
    }

    QVERIFY(ast != nullptr);
    QCOMPARE(ast->getChildren().size(), 246);
    // no syntax errors
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}

void TestParser::canParseBlizzardJ() {
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

    QCOMPARE(tokens.size(), 84221);
    QCOMPARE(input.size(), 471054);

    QBENCHMARK {
        tokens = scanner.scan(input, true);
    }

    QCOMPARE(tokens.size(), 56039);
    QCOMPARE(input.size(), 471054);

    VJassParser parser;
    VJassAst *ast = nullptr;

    QBENCHMARK {
        ast = parser.parse(tokens);
    }

    QVERIFY(ast != nullptr);
    QCOMPARE(ast->getChildren().size(), 1067);
    // no syntax errors
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}


void TestParser::canParseNestedExpression() {
    const QString input =
            QString("function bla takes nothing returns nothing\n")
            + "local integer x = -((50 - R2I(10.0)) * ModuloInteger(10, 5) / 3) + (10)\n"
            + "endfunction";

    VJassScanner scanner;

    QList<VJassToken> tokens;
    tokens = scanner.scan(input, false);
    VJassParser parser;
    VJassAst *ast = parser.parse(tokens);

    QVERIFY(ast != nullptr);
    QCOMPARE(ast->getChildren().size(), 1);
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}

void TestParser::canParseSetStatement() {
    const QString input =
            QString("function bla takes nothing returns nothing\n")
            + "set x[10] = 10\n"
            + "endfunction";

    VJassScanner scanner;

    QList<VJassToken> tokens;
    tokens = scanner.scan(input, false);
    VJassParser parser;
    VJassAst *ast = parser.parse(tokens);

    QVERIFY(ast != nullptr);
    QCOMPARE(ast->getChildren().size(), 1);
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}

void TestParser::canParseIfStatement() {
    const QString input =
            QString("function bla takes nothing returns nothing\n")
            + "if true == false then\n"
            + "elseif 10 > 100 then\n"
            + "elseif (10 > 100 or (bla(100) and variable)) then\n"
            + "else\n"
            + "endif\n"
            + "endfunction";

    VJassScanner scanner;

    QList<VJassToken> tokens;
    tokens = scanner.scan(input, false);
    VJassParser parser;
    VJassAst *ast = parser.parse(tokens);

    QVERIFY(ast != nullptr);
    QCOMPARE(ast->getChildren().size(), 1);
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}

void TestParser::canParseCallStatement() {
    const QString input =
            QString("function bla takes nothing returns nothing\n")
            + "call Bla(identifier)\n"
            + "endfunction";

    VJassScanner scanner;

    QList<VJassToken> tokens;
    tokens = scanner.scan(input, false);
    VJassParser parser;
    VJassAst *ast = parser.parse(tokens);

    QVERIFY(ast != nullptr);
    QCOMPARE(ast->getChildren().size(), 1);
    QCOMPARE(ast->getParseErrors().size(), 0);

    delete ast;
    ast = nullptr;
}

QTEST_MAIN(TestParser)
