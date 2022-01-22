#include <QtGui>
#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "vjassscanner.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , popup(new AutoCompletionPopup)
{
    ui->setupUi(this);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

    connect(ui->actionComplete, &QAction::triggered, this, &MainWindow::updateSyntaxErrorsWithAutoComplete);

    connect(ui->actionEnableSyntaxHighlighting, &QAction::changed, this, &MainWindow::updateSyntaxErrorsOnly);
    connect(ui->actionEnableSyntaxCheck, &QAction::changed, this, &MainWindow::updateSyntaxErrorsOnly);

    connect(ui->actionBaradesVJassIDE, &QAction::triggered, this, &MainWindow::aboutDialog);

    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrorsOnly);

    connect(popup, &QTreeWidget::clicked, this, &MainWindow::clickPopupItem);

    connect(ui->outputListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::outputListItemDoubleClicked);

    updateSyntaxErrorsOnly();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete popup;
}

void MainWindow::newFile() {
    ui->textEdit->clear();
}

void MainWindow::openFile() {
    QFileDialog::getOpenFileContent(tr("All files (*.*);;JASS script (*.j *.ai)"), [this](QString name, QByteArray byteArray) {
        this->ui->textEdit->setText(byteArray);
    });
}

void MainWindow::saveAs() {
    QFileDialog::saveFileContent(ui->textEdit->toPlainText().toUtf8(), "myscrypt.j");
}

void MainWindow::highlightTokens(const QList<VJassToken> &tokens) {
    qDebug() << "Tokens size:" << tokens.size();

    // make sure no slots are triggered by this to prevent endless recursions
    disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrorsOnly);

    QTextCharFormat fmtNormal;
    fmtNormal.setBackground(Qt::white);
    fmtNormal.setFontWeight(QFont::Normal);

    QTextCursor cursor(ui->textEdit->document());
    // start at the beginning of the document
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);

    int line = 0;
    int column = 0;

    for (const VJassToken &token : tokens) {
        // only hightlight certain tokens at all
        if (token.getType() != VJassToken::WhiteSpace && token.getType() != VJassToken::LineBreak) {
            // move to token
            int lines = token.getLine() - line;
            int columns = 0;

            if (lines > 0) {
                cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
                cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, lines);
                column = token.getColumn();
                columns = token.getColumn();
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, columns);
                //qDebug() << "Moving to the start of the line";
                //qDebug() << "Moving down" << lines << "lines.";
            } else {
                columns = token.getColumn() - column;
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, columns);
                //qDebug() << "Moving to the right" << column << "columns.";
            }

            //qDebug() << "After moving selection to token to format normal, selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();
            if (lines > 0 || columns > 0) {
                // format normal until here since it might be some space without any tokens
                cursor.setCharFormat(fmtNormal);
                cursor.setPosition(cursor.position(), QTextCursor::MoveAnchor);
            }

            //qDebug() << "After moving selection to token, selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();

            // specify the format for the token itself
            QTextCharFormat fmt;

            if (token.isValidKeyword()) {
                fmt.setForeground(Qt::black);
                fmt.setFontWeight(QFont::Bold);
            } else if (token.getType() == VJassToken::Comment) {
                fmt.setForeground(Qt::gray);
                fmt.setFontItalic(true);
            } else if (token.getType() == VJassToken::BooleanLiteral) {
                fmt.setForeground(Qt::blue);
            } else if (token.getType() == VJassToken::RawCodeLiteral || token.getType() == VJassToken::IntegerLiteral || token.getType() == VJassToken::RealLiteral) {
                fmt.setForeground(Qt::darkYellow);
            } else {
                fmt = fmtNormal;
            }

            //qDebug() << "Before selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();

            // move to the token's end but keep the selection and format it
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, token.getValue().length());
            cursor.setCharFormat(fmt);
            // move anchor to the end
            cursor.setPosition(cursor.position(), QTextCursor::MoveAnchor);

            //qDebug() << "Token:" << token.getValue() << "with line:" << token.getLine() << " and column:" << token.getColumn() << "and token length:" << token.getValue().length();
            //qDebug() << "After selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();

            // TODO set properly if the token contains line breaks
            // update current line and column
            line = token.getLine();
            column = token.getColumn() + token.getValue().length();
        }
    }

    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrorsOnly);
}

void MainWindow::outputListItemDoubleClicked(QListWidgetItem *item) {
    if (item->data(Qt::UserRole).isValid() && item->data(Qt::UserRole).canConvert<QPoint>()) {
        int line = item->data(Qt::UserRole).toPoint().x();
        int column = item->data(Qt::UserRole).toPoint().y();

        QTextCursor cursor(ui->textEdit->document());
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, column);
        ui->textEdit->setTextCursor(cursor);
        ui->textEdit->setFocus();

        qDebug() << "Moving cursor to line" << line << "and column" << column;
    }
}

void MainWindow::updateSyntaxErrors(bool checkSyntax, bool autoComplete, bool highlight) {
    if (checkSyntax || autoComplete || highlight) {
        // TODO Scan and create AST asynchronously so the GUI is not blocked all the time! Only apply it if it has not changed
        VJassScanner scanner;
        QList<VJassToken> tokens = scanner.scan(ui->textEdit->toPlainText(), true);

        if (highlight) {
            qDebug() << "Highlight!";
            highlightTokens(tokens);
        }

        if (checkSyntax || autoComplete) {
            VJassAst ast = this->vjassParser.parse(ui->textEdit->toPlainText(), tokens);

            ui->outputListWidget->clear();

            for (VJassParseError &parseError : ast.getAllParseErrors()) {
                QListWidgetItem *item = new QListWidgetItem(tr("Syntax error at line %1 and column %2: %3").arg(parseError.getLine() + 1).arg(parseError.getColumn() + 1).arg(parseError.getError()));
                item->setData(Qt::UserRole, QPoint(parseError.getLine(), parseError.getColumn()));
                ui->outputListWidget->addItem(item);
            }

            if (ui->outputListWidget->count() == 0) {
                ui->outputListWidget->addItem("No syntax errors.");
            }

            if (autoComplete && ast.getCodeCompletionSuggestions().size() > 0) {
                popup->clear();
                popup->setFocusProxy(this);

                for (VJassAst *codeCompletionSuggestion : ast.getCodeCompletionSuggestions()) {
                    new QTreeWidgetItem(popup, QStringList(codeCompletionSuggestion->toString()));
                }

                qDebug() << "Bottom right 1:" << ui->textEdit, ui->textEdit->cursorRect().bottomRight();
                qDebug() << "Bottom right 2:" << ui->textEdit->mapToGlobal(ui->textEdit->cursorRect().bottomRight());

                popup->move(ui->textEdit->mapToGlobal(ui->textEdit->cursorRect().bottomRight()));

                popup->show();
            }
        }
    }

    if (!highlight) {
        // remove all hightlighting
        disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrorsOnly);

        QTextCursor cursor(ui->textEdit->document());
        QTextCharFormat fmtNormal;
        fmtNormal.setBackground(Qt::white);
        fmtNormal.setFontWeight(QFont::Normal);
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.setCharFormat(fmtNormal);

        connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrorsOnly);
    }
}

void MainWindow::updateSyntaxErrorsOnly() {
    updateSyntaxErrors(ui->actionEnableSyntaxCheck->isChecked(), false, ui->actionEnableSyntaxHighlighting->isChecked());
}

void MainWindow::updateSyntaxErrorsWithAutoComplete() {
    updateSyntaxErrors(ui->actionEnableSyntaxCheck->isChecked(), true, ui->actionEnableSyntaxHighlighting->isChecked());
}

void MainWindow::clickPopupItem(const QModelIndex &index) {
    if (index.row() == popup->topLevelItemCount() - 1) {
        popup->close();
    } else {
        // TODO backtrack and check if the start of the data is already there and only append missing stuff
        ui->textEdit->insertPlainText(index.data().toString());
    }

    popup->close();
}

void MainWindow::aboutDialog() {
    // TODO Show about dialog
}
