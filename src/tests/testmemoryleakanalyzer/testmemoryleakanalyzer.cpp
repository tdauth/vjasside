#include "testmemoryleakanalyzer.h"
#include "vjassscanner.h"
#include "vjassparser.h"
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
    MemoryLeakAnalyzer memoryLeakAnalyzer(ast);

    QCOMPARE(memoryLeakAnalyzer.getGlobals().size(), 1);
}

QTEST_MAIN(TestMemoryLeakAnalyzer)
