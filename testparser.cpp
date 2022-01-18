#include "testparser.h"

void TestParser::canParseFunction()
{
    QString str = "Hello";
    QVERIFY(str.toUpper() == "HELLO");
}
