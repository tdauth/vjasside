#include <QtGui>
#include <QtWidgets>

#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QPlainTextEdit *plainTextEdit, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog),
    plainTextEdit(plainTextEdit)
{
    ui->setupUi(this);

    connect(ui->buttonBox->button(QDialogButtonBox::Close), &QPushButton::pressed, this, &FindDialog::close);

    connect(ui->lineEditSearchExpression, &QLineEdit::textChanged, this, &FindDialog::setSearchExpression);
    connect(ui->pushButtonFindNext, &QPushButton::pressed, this, &FindDialog::findNext);
    connect(ui->pushButtonFindPrevious, &QPushButton::pressed, this, &FindDialog::findPrevious);

    connect(ui->pushButtonReplaceNext, &QPushButton::pressed, this, &FindDialog::replaceNext);
    connect(ui->pushButtonReplaceAll, &QPushButton::pressed, this, &FindDialog::replaceAll);
}

FindDialog::~FindDialog()
{
    delete ui;
}

bool FindDialog::isCaseSensitive() const {
    return ui->checkBoxCaseSensitive->isChecked();
}

bool FindDialog::isRegularExpression() const {
    return ui->checkBoxRegex->isChecked();
}

void FindDialog::setSearchExpression(const QString &expression) {
    QSignalBlocker signalBlocker(ui->lineEditSearchExpression);

    ui->lineEditSearchExpression->setText(expression);
}

bool FindDialog::find(bool forward) {
    QString expression = ui->lineEditSearchExpression->text();

    qDebug() << "Search for" << expression;

    QTextDocument::FindFlags options = QTextDocument::FindFlags();

    if (isCaseSensitive()) {
        options |= QTextDocument::FindCaseSensitively;
    }

    if (!forward) {
        options |= QTextDocument::FindBackward;
    }

    bool result = false;

    if (isRegularExpression()) {
        QRegularExpression regularExpression;
        regularExpression.setPattern(expression);

        result = plainTextEdit->find(regularExpression, options);
    } else {
        result = plainTextEdit->find(expression, options);
    }

    if (!result) {
        QMessageBox::information(this, tr("Nothing found"), tr("Found no matching result."));
    }

    return result;
}

bool FindDialog::findNext() {
    return find(true);
}

bool FindDialog::findPrevious() {
    return find(false);
}

int FindDialog::replace(int startPosition, int maxMatches) {
    const QString searchExpression = ui->lineEditSearchExpression->text();
    const int searchExpressionLength = searchExpression.size();
    const QString replacementText = ui->lineEditReplacementText->text();
    const bool regex = isRegularExpression();
    const bool caseSensitive = isCaseSensitive();

    QRegularExpression regularExpression;

    if (regex) {
        regularExpression.setPattern(searchExpression);

        if (!regularExpression.isValid()) {
            QMessageBox::warning(this, tr("Error"), tr("Invalid regular expression."));

            return 0;
        }
    }

    QTextCursor textCursor = plainTextEdit->textCursor();
    textCursor.setPosition(startPosition);
    int matches = 0;

    while (!textCursor.atEnd() && (maxMatches == 0 || matches < maxMatches)) {
        if (regex) {
            QRegularExpressionMatch match = regularExpression.match(plainTextEdit->document()->toPlainText().mid(textCursor.position()));

            if (match.hasMatch()) {
                textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, match.capturedLength());
                textCursor.removeSelectedText();
                textCursor.insertText(replacementText);
                textCursor.setPosition(textCursor.selectionEnd());
                matches++;
            } else {
                textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
            }

            QTextCursor plainTextEditCursor = plainTextEdit->textCursor();
            plainTextEditCursor.setPosition(textCursor.position());
        } else {
            textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, searchExpressionLength);
            QString compareTo = textCursor.selectedText();

            if ((caseSensitive && searchExpression == compareTo) || (!caseSensitive && searchExpression.toLower() == compareTo.toLower())) {
                textCursor.removeSelectedText();
                textCursor.insertText(replacementText);
                textCursor.setPosition(textCursor.selectionEnd());
                matches++;
            } else {
                textCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
                QTextCursor plainTextEditCursor = plainTextEdit->textCursor();
                plainTextEditCursor.setPosition(textCursor.position());
            }

            QTextCursor plainTextEditCursor = plainTextEdit->textCursor();
            plainTextEditCursor.setPosition(textCursor.position());
        }
    }

    QMessageBox::information(this, tr("Replaced"), tr("Replaced %1 times.").arg(matches));

    return matches;
}

int FindDialog::replaceNext() {
    return replace(plainTextEdit->textCursor().position(), 1);
}

int FindDialog::replaceAll() {
    return replace(0, 0);
}
