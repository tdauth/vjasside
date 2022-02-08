#ifndef TESTHIGHLIGHTINFO_H
#define TESTHIGHLIGHTINFO_H

#include <QTest>

class TestHighlightInfo : public QObject
{
    Q_OBJECT

    private slots:
        void canHoldTokens();
        void canHoldTokensFromCommonJ();
        void canHoldTokensFromBlizzardJ();
        //void canOrderCodeElementsFromCommonJ();

        void canHoldAst();
};

#endif // TESTHIGHLIGHTINFO_H
