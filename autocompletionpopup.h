#ifndef AUTOCOMPLETIONPOPUP_H
#define AUTOCOMPLETIONPOPUP_H

#include <QTreeWidget>

class AutoCompletionPopup : public QTreeWidget
{
public:
    AutoCompletionPopup();

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
};

#endif // AUTOCOMPLETIONPOPUP_H
