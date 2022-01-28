#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QPlainTextEdit>

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent);
    virtual ~TextEdit();

protected:
    virtual void keyPressEvent(QKeyEvent *e) override;
};

#endif // TEXTEDIT_H
