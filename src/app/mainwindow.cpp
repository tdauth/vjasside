#include <QtGui>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "vjassscanner.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , popup(new AutoCompletionPopup)
    , timerId(0)
    , timerIdCheck(startTimer(3000))
    , scanAndParseInput(nullptr)
    , scanAndParseResults(nullptr)
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

    // whenever the user changes the text we have to wait with our highlighting and syntax check for some time to prevent blocking the GUI all the time
    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);

    connect(popup, &QTreeWidget::clicked, this, &MainWindow::clickPopupItem);

    connect(ui->outputListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::outputListItemDoubleClicked);

    scanAndParseThread = QThread::create([this]() {
                VJassScanner scanner;
                VJassParser parser;

                // consume the text forever and scan and parse it
                while (true) {
                    qDebug() << "Retrieving possible scan and parse input text from thread";

                    QString *text = this->scanAndParseInput.fetchAndStoreAcquire(nullptr);

                    if (text != nullptr) {
                        qDebug() << "Got scan and parse input text";

                        QString input = QString(*text);
                        input.detach(); // avoid memory issues by creating a real deep copy
                        // discard the transfered text
                        delete text;
                        text = nullptr;

                        // TODO Allow interrupting by changing scanAndParseInput. Maybe we have to kill the thread and restart it which is also quite expensive.
                        QList<VJassToken> tokens = scanner.scan(input, true);
                        VJassAst *ast = parser.parse(tokens);
                        ScanAndParseResults *results = new ScanAndParseResults(std::move(tokens), ast);

                        // by the end there could be new input and we have to start again
                        if (this->scanAndParseInput.loadAcquire() == nullptr) {
                            qDebug() << "Finished scanning and parsing and storing it";
                            this->scanAndParseResults.storeRelease(results);
                        } else {
                            qDebug() << "Finished scanning and parsing but discarding it";
                            delete results;
                            results = nullptr;
                        }
                    } else {
                        qDebug() << "Waiting for scan and parse input text in thread";
                        QThread::sleep(2);
                    }
                }
    });
    // we have to start the thread and it should not block any applications
    scanAndParseThread->start(QThread::LowestPriority);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete popup;

    killTimer(timerIdCheck);

    if (timerId != 0) {
        killTimer(timerId);
    }

    scanAndParseThread->exit(0);
    scanAndParseThread->wait();
    delete scanAndParseThread;
}

void MainWindow::newFile() {
    ui->textEdit->clear();
}

void MainWindow::openFile() {
    QFileDialog::getOpenFileContent(tr("All files (*.*);;JASS script (*.j *.ai)"), [this](QString /* name */, QByteArray byteArray) {
        this->ui->textEdit->setText(byteArray);
    });
}

void MainWindow::saveAs() {
    QFileDialog::saveFileContent(ui->textEdit->toPlainText().toUtf8(), "myscrypt.j");
}

void MainWindow::highlightTokens(const QList<VJassToken> &tokens) {
    // TODO this method is slow as hell! Improve its speed! Probably too many tokens!
    qDebug() << "Beginning highlighting tokens with tokens size:" << tokens.size();
    QElapsedTimer timer;
    timer.start();

    // make sure no slots are triggered by this to prevent endless recursions
    disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);

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

            // formats are taken from https://github.com/tdauth/syntaxhighlightings/blob/master/Kate/vjass.xml
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
            } else if (token.getType() == VJassToken::Text) {
                // Make a quick check for the symbol from hash sets of standard types and functions so we have these highlighted even without syntax checking
                if (token.isCommonJType()) {
                    fmt.setForeground(Qt::blue);
                } else if (token.isCommonJNative()) {
                    fmt.setForeground(QColor(0xba55d3));
                    fmt.setFontWeight(QFont::Bold);
                } else if (token.isCommonJConstant()) {
                    fmt.setForeground(QColor(0xff7f50));
                    fmt.setFontItalic(true);
                } else {
                    fmt = fmtNormal;
                }
            } else {
                fmt = fmtNormal;
            }

            //qDebug() << "Before selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();

            // move to the token's end but keep the selection and format it
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, token.getValueLength());
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

    // reset formatting for upcoming text
    ui->textEdit->setCurrentCharFormat(fmtNormal);

    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);

    qDebug() << "Ending highlighting tokens with tokens size:" << tokens.size() << "and elapsed time" << timer.elapsed() << "ms and in seconds" << (timer.elapsed() / 1000) << "and in minutes" << (timer.elapsed() / (1000 * 60));
}

void MainWindow::highlightAst(VJassAst *ast) {
    qDebug() << "AST size:" << ast->getChildren().size();
    QStack<VJassParseError> parseErrors;

    for (VJassParseError parseError : ast->getAllParseErrors()) {
        parseErrors.push_back(parseError);
    }

    // make sure no slots are triggered by this to prevent endless recursions
    disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);

    QTextCharFormat fmtNormal;
    fmtNormal.setBackground(Qt::white);
    fmtNormal.setFontWeight(QFont::Normal);

    QTextCursor cursor(ui->textEdit->document());
    // start at the beginning of the document
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);

    int line = 0;
    int column = 0;

    while (!parseErrors.isEmpty()) {
        VJassParseError parseError = parseErrors.pop();

        // move to token
        int lines = parseError.getLine() - line;
        int columns = 0;

        if (lines > 0) {
            cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, lines);
            column = parseError.getColumn();
            columns = parseError.getColumn();
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, columns);
            //qDebug() << "Moving to the start of the line";
            //qDebug() << "Moving down" << lines << "lines.";
        } else {
            columns = parseError.getColumn() - column;
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, columns);
            //qDebug() << "Moving to the right" << column << "columns.";
        }

        //qDebug() << "After moving selection to token to format normal, selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();
        cursor.setPosition(cursor.position(), QTextCursor::MoveAnchor);

        //qDebug() << "After moving selection to token, selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();

        // specify the format for the token itself
        QTextCharFormat fmt;
        fmt.setUnderlineColor(Qt::red);
        fmt.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

        //qDebug() << "Before selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();

        // move to the token's end but keep the selection and format it
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, parseError.getError().length());
        cursor.setCharFormat(fmt);
        // move anchor to the end
        cursor.setPosition(cursor.position(), QTextCursor::MoveAnchor);

        //qDebug() << "Token:" << token.getValue() << "with line:" << token.getLine() << " and column:" << token.getColumn() << "and token length:" << token.getValue().length();
        //qDebug() << "After selection start:" << cursor.selectionStart() << "and selection end" << cursor.selectionEnd();

        // TODO set properly if the token contains line breaks
        // update current line and column
        line = parseError.getLine();
        column = parseError.getColumn() + parseError.getError().length();
    }

    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);
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
    if (!checkSyntax && !autoComplete && !highlight) {
        clearAllHighLighting();
    }
}

void MainWindow::updateSyntaxErrorsOnly() {
    updateSyntaxErrors(ui->actionEnableSyntaxCheck->isChecked(), false, ui->actionEnableSyntaxHighlighting->isChecked());
}

void MainWindow::updateSyntaxErrorsWithAutoComplete() {
    updateSyntaxErrors(ui->actionEnableSyntaxCheck->isChecked(), true, ui->actionEnableSyntaxHighlighting->isChecked());
}

void MainWindow::clickPopupItem(const QModelIndex &index) {
    const QString &keyword = index.data().toString();

    // backtrack and check if the start of the data is already there and only append missing stuff
    QTextCursor cursor(ui->textEdit->document());
    const int start = ui->textEdit->textCursor().position();
    cursor.setPosition(start);
    bool found = false;

    for (int i = start; i > 0 && !found && start - i < keyword.length(); i--) {
        qDebug() << "Move cursor back";
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);

        if (keyword.startsWith(cursor.selectedText())) {
            cursor.removeSelectedText();
            cursor.insertText(keyword);

            found = true;
        }
    }

    // backtracking did not replace anything, so just add the text
    if (!found) {
        ui->textEdit->insertPlainText(keyword);
    }

    popup->close();
}

void MainWindow::aboutDialog() {
    QMessageBox::about(this, tr("Baradé's vJass IDE"), tr("Integrated development environment for the scripting languages JASS and vJass from the computer game Warcraft III. This is an Open Source application based on the Qt framework. Visit <a href=\"https://github.com/tdauth/vjasside\">the GitHub repository</a> to contribute or star it."));
}

void MainWindow::restartTimer() {
    if (timerId != 0) {
        killTimer(timerId);
    }

    // create a deep copy to avoid data races
    QString text = ui->textEdit->toPlainText();
    text.detach();

    qDebug() << "Storing text with length" << text.length() << "for the thread and starting user input timer";

    //qassert(text.isDetached());
    scanAndParseInput.storeRelease(new QString(text));
    scanAndParseResults.storeRelease(nullptr);

    timerId = startTimer(3000);
}

void MainWindow::clearAllHighLighting() {
    disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);

    QTextCursor cursor(ui->textEdit->document());
    QTextCharFormat fmtNormal;
    fmtNormal.setBackground(Qt::white);
    fmtNormal.setFontWeight(QFont::Normal);
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.setCharFormat(fmtNormal);

    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (event->timerId() == timerId) {
        timerId = 0;
    } else if (timerId == 0 && event->timerId() == timerIdCheck) {
        qDebug() << "Checking scan and parse result";

        ScanAndParseResults *scanAndParseResults = this->scanAndParseResults.fetchAndStoreAcquire(nullptr);

        // if there are results we will highlight everything now
        if (scanAndParseResults != nullptr) {
            qDebug() << "Got scan and parse result from thread into the main window";

            bool checkSyntax = ui->actionEnableSyntaxCheck->isChecked();
            bool autoComplete = false;
            bool highlight = ui->actionEnableSyntaxHighlighting->isChecked();

            if (checkSyntax || autoComplete || highlight) {
                if (highlight) {
                    qDebug() << "Highlight!";
                    highlightTokens(scanAndParseResults->tokens);
                }

                if (checkSyntax || autoComplete) {
                    ui->outputListWidget->clear();

                    for (const VJassParseError &parseError : scanAndParseResults->ast->getAllParseErrors()) {
                        QListWidgetItem *item = new QListWidgetItem(tr("Syntax error at line %1 and column %2: %3").arg(parseError.getLine() + 1).arg(parseError.getColumn() + 1).arg(parseError.getError()));
                        item->setData(Qt::UserRole, QPoint(parseError.getLine(), parseError.getColumn()));
                        ui->outputListWidget->addItem(item);
                    }

                    if (ui->outputListWidget->count() == 0) {
                        ui->outputListWidget->addItem("No syntax errors.");
                    }

                    if (highlight) {
                        // TODO highlight the AST together with the tokens in one single function to improve the performance and the highlighting of Qt.
                        highlightAst(scanAndParseResults->ast);
                    }

                    if (autoComplete && scanAndParseResults->ast->getCodeCompletionSuggestions().size() > 0) {
                        popup->clear();
                        popup->setFocusProxy(this);
                        QTreeWidgetItem *firstItem = nullptr;

                        for (VJassAst *codeCompletionSuggestion : scanAndParseResults->ast->getCodeCompletionSuggestions()) {
                            QTreeWidgetItem *item = new QTreeWidgetItem(popup, QStringList(codeCompletionSuggestion->toString()));

                            if (firstItem == nullptr) {
                                firstItem = item;
                            }
                        }

                        qDebug() << "Bottom right 1:" << ui->textEdit, ui->textEdit->cursorRect().bottomRight();
                        qDebug() << "Bottom right 2:" << ui->textEdit->mapToGlobal(ui->textEdit->cursorRect().bottomRight());

                        popup->move(ui->textEdit->mapToGlobal(ui->textEdit->cursorRect().bottomRight()));
                        // preselect first entry
                        popup->setCurrentItem(firstItem);

                        popup->show();
                    }
                }
            }

            delete scanAndParseResults;
            scanAndParseResults = nullptr;
        }
    }
}
