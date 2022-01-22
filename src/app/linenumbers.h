#ifndef LINENUMBERS_H
#define LINENUMBERS_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class LineNumbers; }
QT_END_NAMESPACE

class LineNumbers : public QWidget
{
    Q_OBJECT

public:
    LineNumbers(QWidget *parent);
    virtual ~LineNumbers();

private:
    Ui::LineNumbers *ui;
};

#endif // LINENUMBERS_H
