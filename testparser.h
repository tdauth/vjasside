#ifndef TESTPARSER_H
#define TESTPARSER_H

#include <QTest>

class TestParser : public QObject
{
    Q_OBJECT

    private slots:
        void canParseFunction();
};

#endif // TESTPARSER_H
