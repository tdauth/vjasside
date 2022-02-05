#include "testmemoryleakanalyzer.h"
#include "vjassscanner.h"
#include "vjassparser.h"
#include "vjassglobals.h"
#include "memoryleakanalyzer.h"

void TestMemoryLeakAnalyzer::canDetectGlobalsLeaks() {
    QString input = QString("globals\n")
            + "\tlocation bla = Location(0.0, 0.0)\n"
            + "endglobals\n"
            ;
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(input, true);
    VJassParser parser;
    VJassAst *ast = parser.parse(tokens);

    qDebug() << "AST: " << ast->toString();
    qDebug() << "AST parse errors: ";

    for (const VJassParseError &error : ast->getAllParseErrors()) {
        qDebug() << "Error:" << error.getError();
    }

    QCOMPARE(ast->getChildren().size(), 1);
    QCOMPARE(typeid(*ast->getChildren().at(0)), typeid(VJassGlobals));
    QCOMPARE(ast->getChildren().at(0)->getChildren().size(), 1);
    QCOMPARE(typeid(*ast->getChildren().at(0)->getChildren().at(0)), typeid(VJassGlobal));
    QCOMPARE(ast->getAllParseErrors().size(), 0);

    MemoryLeakAnalyzer memoryLeakAnalyzer(ast);

    QCOMPARE(memoryLeakAnalyzer.getGlobals().size(), 1);
    QCOMPARE(memoryLeakAnalyzer.getGlobals().at(0)->getName(), "bla");
}

QTEST_MAIN(TestMemoryLeakAnalyzer)
