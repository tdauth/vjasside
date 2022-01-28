#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QWidget>
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class TextEdit; }
QT_END_NAMESPACE

class TextEdit : public QWidget
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent);
    virtual ~TextEdit();

    QPlainTextEdit* getPlainTextEdit() const;

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::TextEdit *ui;
};

#endif // TEXTEDIT_H
