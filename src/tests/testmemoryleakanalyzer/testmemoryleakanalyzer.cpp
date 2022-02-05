#include "testmemoryleakanalyzer.h"
#include "vjassscanner.h"
#include "vjassparser.h"
#include "vjassglobals.h"
#include "memoryleakanalyzer.h"

void TestMemoryLeakAnalyzer::canDetectLeaks() {
    QFile f("wc3reforged/leaking.j");

    QVERIFY(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString input = in.readAll();

    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(input, true);
    VJassParser parser;
    VJassAst *ast = parser.parse(tokens);

    qDebug() << "AST: " << ast->toString();
    qDebug() << "AST parse errors: ";

    for (const VJassParseError &error : ast->getAllParseErrors()) {
        qDebug() << "Error:" << error.getError();
    }

    QCOMPARE(ast->getAllParseErrors().size(), 0);

    MemoryLeakAnalyzer memoryLeakAnalyzer(ast);

    QCOMPARE(memoryLeakAnalyzer.getGlobals().size(), 1);
    QCOMPARE(memoryLeakAnalyzer.getGlobals().at(0)->getName(), "bla");
}

void TestMemoryLeakAnalyzer::canDetectNoLeaks() {
    QFile f("wc3reforged/notleaking.j");

    QVERIFY(f.open(QFile::ReadOnly | QFile::Text));

    QTextStream in(&f);
    QString input = in.readAll();
}

QTEST_MAIN(TestMemoryLeakAnalyzer)
