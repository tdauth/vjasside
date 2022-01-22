#include <QtTest>

#include "testparser.h"

void TestParser::canParseFunction()
{
    QString str = "Hello";
    QVERIFY(str.toUpper() == "HELLO");
}

//QTEST_MAIN(TestParser)
//#include "testparser.moc"
