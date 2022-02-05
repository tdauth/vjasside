#ifndef TESTMEMORYLEAKANALYZER_H
#define TESTMEMORYLEAKANALYZER_H

#include <QTest>

class TestMemoryLeakAnalyzer : public QObject
{
    Q_OBJECT

    private slots:
        void canDetectLeaks();
        void canDetectNoLeaks();
};

#endif // TESTMEMORYLEAKANALYZER_H
