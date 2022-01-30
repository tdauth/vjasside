#include <QtTest>

#include "../../app/pjass.h"
#include "testpjass.h"

void TestPJass::canRunPJass() {
    PJass pjass;

    int exitCode = -1;

    QBENCHMARK {
        exitCode = pjass.run("function");
    }

    QCOMPARE(0, exitCode);
    QCOMPARE("", pjass.getStandardError());
    QCOMPARE("", pjass.getStandardOutput());

    QCOMPARE(10, PJass::outputToParseErrors(pjass.getStandardError()).size());
}

QTEST_MAIN(TestPJass)
