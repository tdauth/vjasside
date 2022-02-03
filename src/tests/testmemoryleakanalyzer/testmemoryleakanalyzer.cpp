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


    QCOMPARE(ast->getChildren().size(), 1);
    QCOMPARE(typeid(ast->getChildren().at(0)), typeid(VJassGlobals*));
    QCOMPARE(typeid(ast->getChildren().at(0)->getChildren().size()), 1);
    QCOMPARE(typeid(ast->getChildren().at(0)->getChildren().at(0)), typeid(VJassGlobal*));
    QCOMPARE(ast->getAllParseErrors().size(), 0);

    MemoryLeakAnalyzer memoryLeakAnalyzer(ast);

    QCOMPARE(memoryLeakAnalyzer.getGlobals().size(), 1);
}

QTEST_MAIN(TestMemoryLeakAnalyzer)
