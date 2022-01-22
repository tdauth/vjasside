#include <QtGui>

#include "autocompletionpopup.h"

AutoCompletionPopup::AutoCompletionPopup()
{
    this->headerItem()->setText(0, tr("Auto Completion"));
    this->setMinimumSize(QSize(128, 128));
    this->setWindowFlags(Qt::Popup);
    this->setFocusPolicy(Qt::NoFocus);
}

void AutoCompletionPopup::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
    } else if (event->key() == Qt::Key_Enter) {
        qDebug() << "Pressed Enter!";

        QTreeWidgetItem *item = currentItem();

        if (item != nullptr) {
            qDebug() << "Item selected on pressing enter";

            // TODO click the item
            emit itemClicked(item, 0);
        } else {
            qDebug() << "No item selected on pressing enter!";
        }
    }

    QTreeWidget::keyPressEvent(event);
}

void AutoCompletionPopup::mousePressEvent(QMouseEvent *event) {
    //this->close();
    QTreeWidget::mousePressEvent(event);
}

void AutoCompletionPopup::focusOutEvent(QFocusEvent *event) {
    //this->close();
    QTreeWidget::focusOutEvent(event);
}
