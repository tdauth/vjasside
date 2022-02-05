#include <QtGui>
#include <QtWidgets>

#include "textedit.h"

TextEdit::TextEdit(QWidget *parent) : QPlainTextEdit(parent), pressedControl(false) {
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // remove the space above the first line
    document()->setDocumentMargin(0.0);
}

TextEdit::~TextEdit() {
}

void TextEdit::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Backtab) {
            QTextCursor cur = textCursor();

            if (!cur.selection().isEmpty()) {
                const bool back = e->key() == Qt::Key_Backtab;
                int a = cur.anchor();
                int p = cur.position();

                cur.setPosition(a);
                cur.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
                a = cur.position();
                // save a new anchor at the beginning of the line of the selected text
                cur.setPosition(a);
                cur.setPosition(p, QTextCursor::KeepAnchor);
                // set a new selection with the new beginning
                // TODO Not only take the selection but the text from ALL blocks (even if the leading tab are not selected. Otherwise indenting back does not work.
                QString str = cur.selection().toPlainText();
                QStringList list = str.split("\n");
                // get the selected text and split into lines

                for (int i = 0; i < list.count(); i++) {
                    if (back) {
                        if (list[i].startsWith("\t")) {
                            list[i] = list[i].mid(1);
                        }
                    } else {
                        list[i] = QString("\t") + list[i];
                    }
                }
                //insert a space at the beginning of each line

                str = list.join("\n");
                cur.removeSelectedText();
                cur.insertText(str);
                // put the new text back
                cur.setPosition(a);
                cur.setPosition(p, QTextCursor::KeepAnchor);
                // reselect the text for more indents

                setTextCursor(cur);
                // put the whole thing back into the main text
            } else {
                QPlainTextEdit::keyPressEvent(e);
            }
    } else if (e->key() == Qt::Key_Enter) {
        // add spaces to the same level as the line before
        QTextCursor cur = textCursor();
        //int a = cur.anchor();
        //int p = cur.position();
        QString blockText = cur.block().text();

        if (!blockText.isEmpty()) {
            QRegularExpression regularExpression("^([\t ]*)");
            QRegularExpressionMatch regularExpressionMatch = regularExpression.match(blockText);
            QString leadingSpaces = regularExpressionMatch.captured(1);
            cur.insertText("\n");
            cur.movePosition(QTextCursor::Down);
            cur.insertText(leadingSpaces);
        }
    } else if (e->key() == Qt::Key_Control) {
        pressedControl = true;
    } else {
        QPlainTextEdit::keyPressEvent(e);
    }
}

void TextEdit::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Control) {
        pressedControl = false;
    } else {
        QPlainTextEdit::keyReleaseEvent(e);
    }
}
