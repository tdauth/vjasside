#ifndef TESTMAINWINDOW_H
#define TESTMAINWINDOW_H

#include <QTest>

class TestMainWindow : public QObject
{
    Q_OBJECT

    private slots:
        void canHighlight();
};

#endif // TESTMAINWINDOW_H
