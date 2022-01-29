#ifndef TESTPARSER_H
#define TESTPARSER_H

#include <QTest>

class TestParser : public QObject
{
    Q_OBJECT

    private slots:
        void canParseCommonJ();
        void canParseCommonAI();
        void canParseBlizzardJ();
};

#endif // TESTPARSER_H
