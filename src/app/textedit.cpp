#include <QtGui>
#include <QtWidgets>

#include "textedit.h"
#include "ui_textedit.h"

TextEdit::TextEdit(QWidget *parent) : QWidget(parent), ui(new Ui::TextEdit) {
    ui->setupUi(this);
}

TextEdit::~TextEdit() {

}

QPlainTextEdit* TextEdit::getPlainTextEdit() const {
    return ui->plainTextEdit;
}

void TextEdit::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Tab) {
            const bool back = e->modifiers() & Qt::ShiftModifier;

            QTextCursor cur = getPlainTextEdit()->textCursor();
            int a = cur.anchor();
            int p = cur.position();

            cur.setPosition(a);
            cur.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
            a = cur.position();
            // save a new anchor at the beginning of the line of the selected text
            cur.setPosition(a);
            cur.setPosition(p, QTextCursor::KeepAnchor);
            // set a new selection with the new beginning
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

            getPlainTextEdit()->setTextCursor(cur);
            // put the whole thing back into the main text
    }
}
