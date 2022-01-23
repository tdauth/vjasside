#ifndef TESTCODEELEMENTHOLDER_H
#define TESTCODEELEMENTHOLDER_H

#include <QTest>

class TestCodeElementHolder : public QObject
{
    Q_OBJECT

    private slots:
        void canHoldTokens();
        //void canOrderCodeElementsFromCommonJ();
};

#endif // TESTCODEELEMENTHOLDER_H
