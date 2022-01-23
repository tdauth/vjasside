#include <QtTest>

#include "../../app/mainwindow.h"
#include "../../app/vjassscanner.h"
#include "../../app/vjassparser.h"
#include "../../app/highlightinfo.h"
#include "testmainwindow.h"

void TestMainWindow::canHighlight() {
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

    HighLightInfo codeElementHolder(tokens, ast);

    MainWindow mainWindow;
    mainWindow.show();
    mainWindow.pauseParserThread(); // no automatic highlighting

    QBENCHMARK {
        mainWindow.highlightTokensAndAst(codeElementHolder, true);
    }

    delete ast;
    ast = nullptr;
}

QTEST_MAIN(TestMainWindow)
