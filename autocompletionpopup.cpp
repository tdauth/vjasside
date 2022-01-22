#include <QtGui>

#include "autocompletionpopup.h"

AutoCompletionPopup::AutoCompletionPopup()
{
    this->headerItem()->setText(0, tr("Auto Completion"));
    this->setMinimumSize(QSize(128, 128));
    this->setWindowFlags(Qt::Popup);
    //this->setFocusPolicy(Qt::NoFocus);
}

void AutoCompletionPopup::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
}

void AutoCompletionPopup::mousePressEvent(QMouseEvent *event) {
    //this->close();
}

void AutoCompletionPopup::focusOutEvent(QFocusEvent *event) {
    //this->close();
}
