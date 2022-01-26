#include <QtGui>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "vjassscanner.h"
#include "highlightinfo.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , popup(new AutoCompletionPopup)
    , timerId(0)
    , timerIdCheck(startTimer(500)) // poll every 0.5 seconds for a parser result
    , scanAndParseInput(nullptr)
    , scanAndParseResults(nullptr)
    , scanAndParsePaused(0)
{
    ui->setupUi(this);

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::closeFile);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::quit);

    connect(ui->actionLineNumbers, &QAction::changed, this, &MainWindow::updateLineNumbersView);
    connect(ui->actionShowWhiteSpaces, &QAction::changed, this, &MainWindow::showWhiteSpaces);

    connect(ui->actionComplete, &QAction::triggered, this, &MainWindow::updateSyntaxErrorsWithAutoComplete);
    // trigger a restart so the result is updated even if the text has not changed
    connect(ui->actionComplete, &QAction::triggered, this, &MainWindow::restartTimer);

    connect(ui->actionEnableSyntaxHighlighting, &QAction::changed, this, &MainWindow::updateSyntaxErrorsOnly);
    connect(ui->actionEnableSyntaxCheck, &QAction::changed, this, &MainWindow::updateSyntaxErrorsOnly);

    connect(ui->actionBaradesVJassIDE, &QAction::triggered, this, &MainWindow::aboutDialog);

    // whenever the user changes the text we have to wait with our highlighting and syntax check for some time to prevent blocking the GUI all the time
    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::restartTimer);
    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::documentChanges);

    // whenever the user changes the view we have to update the line numbers
    connect(ui->textEdit->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::updateLineNumbers);
    connect(ui->textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::updateLineNumbers);

    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateSelectedLines);

    connect(popup, &QTreeWidget::clicked, this, &MainWindow::clickPopupItem);

    connect(ui->outputListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::outputListItemDoubleClicked);

    // basic settings for text
    ui->textEdit->setFont(HighLightInfo::getNormalFont());
    ui->textEdit->setTabStopDistance(20.0);

    // initial line
    // move cursor to start to edit the document
    ui->textEdit->setFocus();
    updateCursorPosition(0);

    updateLineNumbersView();
    updateLineNumbers();

    syntaxHighlighter = new SyntaxHighlighter(ui->textEdit->document());

    /*
     * Scan, parse and prestore highlighting information concurrently to avoid blocking the GUI.
     */
    scanAndParseThread = QThread::create([this]() {
                VJassScanner scanner;
                VJassParser parser;

                // consume the text forever and scan and parse it
                while (true) {
                    //qDebug() << "Retrieving possible scan and parse input text from thread";

                    if (this->scanAndParsePaused.loadAcquire() == 0) {
                        // consume by replacing it with an empty pointer
                        QString *text = this->scanAndParseInput.fetchAndStoreAcquire(nullptr);

                        if (text != nullptr) {
                            //qDebug() << "Got scan and parse input text";

                            QString input = QString(*text);
                            input.detach(); // avoid memory issues by creating a real deep copy
                            // discard the transfered text
                            delete text;
                            text = nullptr;

                            // TODO Allow interrupting by changing scanAndParseInput. Maybe we have to kill the thread and restart it which is also quite expensive.
                            QList<VJassToken> tokens = scanner.scan(input, true);
                            VJassAst *ast = parser.parse(tokens);
                            qDebug() << "Tokens after scanning" << tokens.size();
                            // this stores also the required highlighting information
                            ScanAndParseResults *results = new ScanAndParseResults(input, std::move(tokens), ast);

                            if (this->scanAndParsePaused.loadAcquire() == 0) {
                                // by the end there could be new input and we have to start again
                                if (this->scanAndParseInput.loadAcquire() == nullptr) {
                                    //qDebug() << "Finished scanning and parsing and storing it";
                                    this->scanAndParseResults.storeRelease(results);
                                } else {
                                    //qDebug() << "Finished scanning and parsing but discarding it";
                                    delete results;
                                    results = nullptr;
                                }
                            } else {
                                //qDebug() << "Finished scanning and parsing but discarding it";
                                delete results;
                                results = nullptr;
                            }
                        } else {
                            QThread::sleep(2);
                        }
                    } else {
                        //qDebug() << "Waiting for scan and parse input text in thread";
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
    ui = nullptr;
    delete popup;
    popup = nullptr;

    killTimer(timerIdCheck);

    if (timerId != 0) {
        killTimer(timerId);
    }

    scanAndParseThread->exit(0);
    scanAndParseThread->wait();
    delete scanAndParseThread;
    scanAndParseThread = nullptr;
}

void MainWindow::newFile() {
    if (documentHasChanged) {
        if (QMessageBox::question(this, tr("Discard unsaved changes"), tr("The document has been modified. Do you want to save your changes?")) == QMessageBox::Yes) {
            if (saveAs()) {
                ui->textEdit->clear();

                documentHasChanged = false;
                updateWindowTitle();
            }
        }
    } else {
        ui->textEdit->clear();
    }
}

void MainWindow::openFile() {
    bool saved = true;

    if (documentHasChanged) {
        saved = false;

        if (QMessageBox::question(this, tr("Discard unsaved changes"), tr("The document has been modified. Do you want to save your changes?")) == QMessageBox::Yes) {
            if (saveAs()) {
                saved = true;
            }
        }
    }

    if (saved) {
        QString openFileName = QFileDialog::getOpenFileName(this, tr("Open File"), fileDir, tr("All files (*.*);;JASS script (*.j *.ai)"));

        if (!openFileName.isEmpty()) {
            QFile f(openFileName);
            QFileInfo fileInfo(openFileName);
            fileDir = fileInfo.absoluteDir().path();

            if (f.open(QIODevice::ReadOnly)) {
                this->ui->textEdit->document()->setPlainText(f.readAll());

                documentHasChanged = false;
                updateWindowTitle();
            } else {
                QMessageBox::warning(this, tr("Error"), tr("Error on reading file into %1").arg(openFileName));
            }
        }
    }
}

bool MainWindow::saveAs() {
    QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save File"), fileDir, tr("All files (*.*);;JASS script (*.j *.ai)"));

    if (!saveFileName.isEmpty()) {
        QFile f(saveFileName);
        QFileInfo fileInfo(saveFileName);
        fileDir = fileInfo.absoluteDir().path();

        if (f.open(QIODevice::WriteOnly)) {
            f.write(ui->textEdit->toPlainText().toUtf8());

            documentHasChanged = false;
            updateWindowTitle();

            return true;
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Error on writing file into %1").arg(saveFileName));
        }
    }

    return false;
}

void MainWindow::closeFile() {
    if (documentHasChanged) {
        if (QMessageBox::question(this, tr("Discard unsaved changes"), tr("The document has been modified. Do you want to save your changes?")) == QMessageBox::Yes) {
            if (saveAs()) {
                ui->textEdit->clear();
            }
        } else {
            ui->textEdit->clear();
        }
    } else {
        ui->textEdit->clear();
    }
}

void MainWindow::quit() {
    if (documentHasChanged) {
        if (QMessageBox::question(this, tr("Discard unsaved changes"), tr("The document has been modified. Do you want to save your changes?")) == QMessageBox::Yes) {
            if (saveAs()) {
                this->close();
            }
        } else {
            this->close();
        }
    } else {
        this->close();
    }
}

void MainWindow::updateCursorPosition(int position) {
    QTextCursor startCursor = ui->textEdit->textCursor();
    startCursor.setCharFormat(HighLightInfo::getNormalFormat());
    startCursor.setPosition(position);
    startCursor.beginEditBlock();
    startCursor.endEditBlock();
    ui->textEdit->setTextCursor(startCursor);
    ui->textEdit->setTabStopDistance(20.0);
}

/**
 * @brief This is one of the most important methods since it does not run concurrently and hence blocks the GUI. It has to be as fast as possible to highlight all code elements.
 *
 * @param highLightInfo Contains a highlighted text document which will replace the current one.
 */
void MainWindow::highlightTokensAndAst(const HighLightInfo &highLightInfo, bool /* checkSyntax */) {
    // TODO this method is slow as hell! Improve its speed! Probably too many tokens!
    // TODO We could try to format a text edit and replace our text edit by the highlighting thread.
    // TODO We can also use setExtraSelections(const QList<QTextEdit::ExtraSelection> &selections) or QSyntaxHighlighter for maybe faster approaches.
    qDebug() << "Beginning highlighting code elements with elements size:" << highLightInfo.getFormattedLocations().size();
    QElapsedTimer timer;
    timer.start();

    const int position = ui->textEdit->textCursor().position();
    qDebug() << "Old cursor position" << position;

    // disable signals
    QSignalBlocker signalBlocker(ui->textEdit);

    // this is the actual slow part
    //QList<QTextEdit::ExtraSelection> extraSelections = highLightInfo.toExtraSelections(ui->textEdit->document(), checkSyntax);

    //qDebug() << "Extra selections" << extraSelections.size();

    //ui->textEdit->setExtraSelections(extraSelections);
    ui->textEdit->setDocument(highLightInfo.getTextDocument());
    // TODO Try setting the format via the syntax highlighter and only the changed blocks
    // TODO Maybe we can detect which block is rehighlighted?
    //syntaxHighlighter->

    // set position back to previous one and remove all formatting
    qDebug() << "Move cursor back to position" << position;
    // TODO Restore the selection as well.
    updateCursorPosition(position);

    /*
    // make sure no slots are triggered by this to prevent endless recursions
    disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);
    disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::documentChanges);

    // directly iterating through all entries is the fastest way
    for (auto iterator = codeElementHolder.getFormattedLocations().constKeyValueBegin(); iterator != codeElementHolder.getFormattedLocations().constKeyValueEnd(); ++iterator) {
        const HighLightInfo::Location &location = iterator->first;
        const HighLightInfo::CustomTextCharFormat &customTextCharFormat = iterator->second;

        // move a cursor to the character
        QTextCursor cursor(ui->textEdit->document());
        cursor.setPosition(0, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, location.line);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, location.column);

        // format all characters
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, customTextCharFormat.length);
        QTextCharFormat fmt = getNormalFormat();
        customTextCharFormat.applyToTextCharFormat(fmt, checkSyntax);
        cursor.setCharFormat(fmt);

        // reset selecting and reset format, otherwise moving the cursor might reformat anything
        cursor.clearSelection();
        cursor.setCharFormat(getNormalFormat());
        ui->textEdit->setCurrentCharFormat(getNormalFormat());
    }

    // reset formatting for upcoming text
    ui->textEdit->setCurrentCharFormat(getNormalFormat());

    // enable triggering text changes by the user again
    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);
    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::documentChanges);
    */

    qDebug() << "Ending highlighting code elements with elements size:" << highLightInfo.getFormattedLocations().size() << "and elapsed time" << timer.elapsed() << "ms and in seconds" << (timer.elapsed() / 1000) << "and in minutes" << (timer.elapsed() / (1000 * 60));
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
    this->expectAutoComplete = autoComplete;

    if (!checkSyntax && !autoComplete && !highlight) {
        //clearAllHighLighting();
    }
}

void MainWindow::updateSyntaxErrorsOnly() {
    updateSyntaxErrors(ui->actionEnableSyntaxCheck->isChecked(), false, ui->actionEnableSyntaxHighlighting->isChecked());
}

void MainWindow::updateSyntaxErrorsWithAutoComplete() {
    updateSyntaxErrors(ui->actionEnableSyntaxCheck->isChecked(), true, ui->actionEnableSyntaxHighlighting->isChecked());
}

void MainWindow::updateLineNumbersView() {
    ui->lineNumbersWidget->setVisible(ui->actionLineNumbers->isChecked());
    updateLineNumbers();
}

void MainWindow::showWhiteSpaces() {
    QTextOption option;

    if (ui->actionShowWhiteSpaces->isChecked()) {
        option.setFlags(QTextOption::ShowLineAndParagraphSeparators | QTextOption::ShowTabsAndSpaces);
    } else {
        option.setFlags(option.flags() & QTextOption::ShowLineAndParagraphSeparators & QTextOption::ShowTabsAndSpaces);
    }

    ui->textEdit->document()->setDefaultTextOption(option);
}

void MainWindow::updateLineNumbers() {
    QTextCursor startCursor = ui->textEdit->cursorForPosition(QPoint(0, 0));
    //const int start_pos = startCursor.position();
    const QPoint bottom_right(ui->textEdit->viewport()->width() - 1, ui->textEdit->viewport()->height() - 1);
    //const QPoint bottom_right(ui->textEdit->viewport()->width(), ui->textEdit->viewport()->height());
    QTextCursor bottomCursor = ui->textEdit->cursorForPosition(bottom_right);
    //const int end_pos = ui->textEdit->cursorForPosition(bottom_right).position();

    const int startLine = startCursor.blockNumber();
    const int visibleLines = bottomCursor.blockNumber() - startCursor.blockNumber() + 1;

    //qDebug() << "Visible lines" << visibleLines << "starting at" << startLine;

    QList<qreal> lineHeights;

    for (int i = 0; i < visibleLines; i++) {
        const qreal lineHeight = ui->textEdit->document()->documentLayout()->blockBoundingRect(startCursor.block()).height(); // TODO is 0 before anything happens
        lineHeights.push_back(qMax<qreal>(16, lineHeight)); // since it can be set a min height
        //lineHeights.push_back(startCursor.blockFormat().lineHeight());
        //lineHeights.push_back(startCursor.charFormat().font().pointSizeF());
        startCursor.movePosition(QTextCursor::Down);
    }

    ui->lineNumbersWidget->setLineNumbers(startLine, visibleLines, lineHeights);
    updateSelectedLines();
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

    qDebug() << "Restarting user input timer";
    // wait 2 seconds after the user has stopped writing something
    timerId = startTimer(2000);

    // TODO Restart the thread, so it will wait for the next user input instead of continuing with the current scanning/parsing/highlighting.
}

void MainWindow::documentChanges() {
    documentHasChanged = true;
    updateWindowTitle();
    updateLineNumbers();
}

void MainWindow::updateWindowTitle() {
    if (documentHasChanged) {
        setWindowTitle(tr("Baradé's vJass IDE *"));
    } else {
        setWindowTitle(tr("Baradé's vJass IDE"));
    }
}

void MainWindow::updateSelectedLines() {
    const int selectionStart = ui->textEdit->textCursor().selectionStart();
    const int selectionEnd = ui->textEdit->textCursor().selectionEnd();
    const int lineStart = ui->textEdit->textCursor().document()->findBlock(selectionStart).blockNumber();
    const int lineEnd = ui->textEdit->textCursor().document()->findBlock(selectionEnd).blockNumber();

    ui->lineNumbersWidget->updateSelectedLines(lineStart, lineEnd);

    const int currentColumn = ui->textEdit->textCursor().positionInBlock();
    const int currentLine = ui->textEdit->textCursor().blockNumber();

    statusBar()->showMessage(tr("Column: %1, Line: %2").arg(currentColumn + 1).arg(currentLine + 1));
}

void MainWindow::pauseParserThread() {
    scanAndParsePaused.storeRelease(1);
}

void MainWindow::resumeParserThread() {
    scanAndParsePaused.storeRelease(0);
}

void MainWindow::clearAllHighLighting() {
    //disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);
    //disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::documentChanges);

    QSignalBlocker signalBlocker(ui->textEdit);
    QTextCursor cursor(ui->textEdit->document());
    QTextCharFormat fmtNormal;
    fmtNormal.setBackground(Qt::white);
    fmtNormal.setFontWeight(QFont::Normal);
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.setCharFormat(fmtNormal);

    //connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::restartTimer);
    //connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::documentChanges);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    // the user input timer finishes, so the user has stopped writing for some time, let's send the finished text to the thread for handling.
    if (event->timerId() == timerId) {
        timerId = 0; // set to 0 so the other timer will fetch from now on

        // create a deep copy to avoid data races
        QString text = ui->textEdit->toPlainText();
        text.detach();

        qDebug() << "Finished user input timer and storing text with length" << text.length() << "for the scan and parser thread";

        //qassert(text.isDetached());
        scanAndParseInput.storeRelease(new QString(text));
        scanAndParseResults.storeRelease(nullptr);
    } else if (timerId == 0 && event->timerId() == timerIdCheck) {
        //qDebug() << "Checking scan and parse result";

        ScanAndParseResults *scanAndParseResults = this->scanAndParseResults.fetchAndStoreAcquire(nullptr);

        // if there are results we will highlight everything now
        if (scanAndParseResults != nullptr) {
            qDebug() << "Got scan and parse result from thread into the main window";

            bool checkSyntax = ui->actionEnableSyntaxCheck->isChecked();
            bool autoComplete = expectAutoComplete;
            expectAutoComplete = false; // reset
            bool highlight = ui->actionEnableSyntaxHighlighting->isChecked();

            if (checkSyntax || autoComplete || highlight) {
                if (highlight) {
                    qDebug() << "Highlight!";
                    highlightTokensAndAst(scanAndParseResults->highLightInfo, checkSyntax);
                }

                if (checkSyntax || autoComplete) {
                    ui->outputListWidget->clear();

                    const QList<VJassParseError> &parseErrors = scanAndParseResults->highLightInfo.getParseErrors();

                    for (const VJassParseError &parseError : parseErrors) {
                        QListWidgetItem *item = new QListWidgetItem(tr("Syntax error at line %1 and column %2: %3").arg(parseError.getLine() + 1).arg(parseError.getColumn() + 1).arg(parseError.getError()));
                        item->setData(Qt::UserRole, QPoint(parseError.getLine(), parseError.getColumn()));
                        ui->outputListWidget->addItem(item);
                    }

                    if (ui->outputListWidget->count() == 0) {
                        ui->outputListWidget->addItem(tr("No syntax errors."));
                        ui->tabWidget->setTabText(0, tr("0 Syntax Errors"));
                    } else {
                        ui->tabWidget->setTabText(0, tr("%n Syntax Errors", "%n Syntax Error", parseErrors.length()));
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

void MainWindow::resizeEvent(QResizeEvent *event) {
    updateLineNumbers();

    QMainWindow::resizeEvent(event);
}
