#include <QtGui>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "vjassscanner.h"
#include "highlightinfo.h"
#include "pjass.h"
#include "memoryleakanalyzer.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , popup(new AutoCompletionPopup)
    , timerId(0)
    , timerIdCheck(startTimer(500)) // poll every 0.5 seconds for a parser result
    , scanAndParseInput(nullptr)
    , scanAndParseResults(nullptr)
    , scanAndParsePaused(0)
    , stopScanAndParseThread(0)
    , syntaxChecker(0) // vjasside
    , parserName("vjasside")
{
    ui->setupUi(this);
    findDialog = new FindDialog(ui->textEdit, this);
    findDialog->hide();
    syntaxHighlighter = new SyntaxHighlighter(ui->textEdit->document());

    // make only the text edit expand
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 0);

    // make the status bar not disappear
    ui->statusbar->addPermanentWidget(new QLabel(tr("")));

    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionClose, &QAction::triggered, this, &MainWindow::closeFile);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::quit);

    connect(ui->actionGoToLine, &QAction::triggered, this, &MainWindow::goToLine);
    connect(ui->actionFindAndReplace, &QAction::triggered, this, &MainWindow::findAndReplace);
    connect(ui->actionApplyColor, &QAction::triggered, this, &MainWindow::applyColor);

    connect(ui->actionCommonj, &QAction::triggered, this, &MainWindow::openCommonj);
    connect(ui->actionCommonAi, &QAction::triggered, this, &MainWindow::openCommonai);
    connect(ui->actionBlizzardj, &QAction::triggered, this, &MainWindow::openBlizzardj);

    updateScriptsActions();

    connect(ui->actionLineNumbers, &QAction::changed, this, &MainWindow::updateLineNumbersView);
    connect(ui->actionShowWhiteSpaces, &QAction::changed, this, &MainWindow::showWhiteSpaces);

    connect(ui->actionComplete, &QAction::triggered, this, &MainWindow::updateSyntaxErrorsWithAutoComplete);
    // trigger a restart so the result is updated even if the text has not changed
    connect(ui->actionComplete, &QAction::triggered, this, &MainWindow::restartTimer);

    // syntax check
    connect(ui->actionEnableSyntaxHighlighting, &QAction::changed, this, &MainWindow::updateSyntaxErrorsOnly);
    connect(ui->actionEnableSyntaxCheck, &QAction::changed, this, &MainWindow::updateSyntaxErrorsOnly);

    connect(ui->actionVJassIDESyntaxChecker, &QAction::triggered, this, &MainWindow::updatePJassSyntaxCheckerVJassIDE);
    connect(ui->actionPJassSyntaxChecker, &QAction::triggered, this, &MainWindow::updatePJassSyntaxCheckerPJass);

    connect(ui->actionAnalyzeMemoryLeaks, &QAction::triggered, this, &MainWindow::setAnalyzeMemoryLeaks);

    connect(ui->actionJASS_Manual, &QAction::triggered, this, &MainWindow::openJASSManual);
    connect(ui->actionCodeOnHive, &QAction::triggered, this, &MainWindow::openCodeOnHive);
    connect(ui->actionPJassUpdates, &QAction::triggered, this, &MainWindow::openPJassUpdates);

    connect(ui->actionAboutPJass, &QAction::triggered, this, &MainWindow::aboutPJassDialog);
    connect(ui->actionBaradesVJassIDE, &QAction::triggered, this, &MainWindow::aboutDialog);

    // whenever the user changes the text we have to wait with our highlighting and syntax check for some time to prevent blocking the GUI all the time
    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::restartTimer);
    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, &MainWindow::documentChanges);

    // whenever the user changes the view we have to update the line numbers
    connect(ui->textEdit->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::updateLineNumbers);
    connect(ui->textEdit->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::updateLineNumbers);

    // whenever the cursor position changes, the selection needs to be updated but also the background color/syntax highlighting
    connect(ui->textEdit, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateSelectedLines);

    connect(popup, &QTreeWidget::clicked, this, &MainWindow::clickPopupItem);

    connect(ui->outputListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::astListItemDoubleClicked);

    // outliner

    connect(ui->checkBoxAll, &QCheckBox::clicked, ui->checkBoxTypes, &QCheckBox::setChecked);
    connect(ui->checkBoxAll, &QCheckBox::clicked, ui->checkBoxNatives, &QCheckBox::setChecked);
    connect(ui->checkBoxAll, &QCheckBox::clicked, ui->checkBoxConstants, &QCheckBox::setChecked);
    connect(ui->checkBoxAll, &QCheckBox::clicked, ui->checkBoxGlobals, &QCheckBox::setChecked);
    connect(ui->checkBoxAll, &QCheckBox::clicked, ui->checkBoxFunctions, &QCheckBox::setChecked);
    connect(ui->checkBoxAll, &QCheckBox::clicked, this, &MainWindow::updateOutliner);

    connect(ui->checkBoxTypes, &QCheckBox::clicked, this, &MainWindow::updateOutliner);
    connect(ui->checkBoxNatives, &QCheckBox::clicked, this, &MainWindow::updateOutliner);
    connect(ui->checkBoxConstants, &QCheckBox::clicked, this, &MainWindow::updateOutliner);
    connect(ui->checkBoxGlobals, &QCheckBox::clicked, this, &MainWindow::updateOutliner);
    connect(ui->checkBoxFunctions, &QCheckBox::clicked, this, &MainWindow::updateOutliner);

    connect(ui->outlinerListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::astListItemDoubleClicked);
    connect(ui->memoryLeaksListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::astListItemDoubleClicked);

    // basic settings for text
    ui->textEdit->setFont(HighLightInfo::getNormalFont());
    ui->textEdit->setTabStopDistance(20.0);
    //ui->textEdit->setPlaceholderText(QFile("wc3reforged/minimaljasssnippet.j").readAll());

    // initial line
    // move cursor to start to edit the document
    ui->textEdit->setFocus();
    updateCursorPosition(0);

    updateLineNumbersView();
    updateLineNumbers();

    // initial document is empty
    resetDocumentChanges();

    /*
     * Scan, parse and prestore highlighting information concurrently to avoid blocking the GUI.
     */
    scanAndParseThread = QThread::create([this]() {
                VJassScanner scanner;
                VJassParser parser;

                // consume the text until the thread is stopped and scan and parse it
                while (this->stopScanAndParseThread.loadAcquire() == 0) {
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

                            QList<VJassToken> tokens = scanner.scan(input, true);
                            qDebug() << "Tokens after scanning" << tokens.size();
                            VJassAst *ast = parser.parse(tokens);
                            QString pjassStandardOutput;
                            QString pjassErrorOutput;

                            // pjass syntax check
                            if (this->syntaxChecker.loadAcquire() == 1) {
                                PJass pjass;
                                int pjassExitCode = pjass.run(input);

                                pjassStandardOutput = pjass.getStandardOutput();
                                pjassErrorOutput = pjass.getStandardError();
                                qDebug() << "Using pjass and getting exit code" << pjassExitCode;
                            }

                            // this stores also the required highlighting information
                            ScanAndParseResults *results = new ScanAndParseResults(input, std::move(tokens), ast, pjassStandardOutput, pjassErrorOutput, this->analyzeMemoryLeaks.loadAcquire() == 1);

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
                            QThread::msleep(300);
                        }
                    } else {
                        //qDebug() << "Waiting for scan and parse input text in thread";
                        QThread::msleep(300);
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

    stopScanAndParseThread.storeRelease(1);
    scanAndParseThread->wait();
    delete scanAndParseThread;
    scanAndParseThread = nullptr;

    if (currentResults != nullptr) {
        delete currentResults;
        currentResults = nullptr;
    }
}

void MainWindow::newFile() {
    if (documentHasChanged) {
        if (QMessageBox::question(this, tr("Discard unsaved changes"), tr("The document has been modified. Do you want to save your changes?")) == QMessageBox::Yes) {
            if (saveAs()) {
                ui->textEdit->clear();
                resetDocumentChanges();
            }
        }
    } else {
        ui->textEdit->clear();
        resetDocumentChanges();
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
                resetDocumentChanges();
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
            resetDocumentChanges();

            return true;
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Error on writing file into %1").arg(saveFileName));
        }
    }

    return false;
}

bool MainWindow::closeFile() {
    if (documentHasChanged) {
        if (QMessageBox::question(this, tr("Discard unsaved changes"), tr("The document has been modified. Do you want to save your changes?")) == QMessageBox::Yes) {
            if (saveAs()) {
                ui->textEdit->clear();
                resetDocumentChanges();
            } else {
                return false;
            }
        } else {
            ui->textEdit->clear();
            resetDocumentChanges();
        }
    } else {
        ui->textEdit->clear();
        resetDocumentChanges();
    }

    return true;
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

void MainWindow::goToLine() {
    bool ok = false;
    const int line = QInputDialog::getInt(this, tr("Go to Line"), tr("Line %1 - %2").arg(1).arg(ui->textEdit->blockCount()), ui->textEdit->textCursor().blockNumber(), 1, ui->textEdit->blockCount(), 1, &ok);

    if (ok) {
        QTextCursor textCursor = ui->textEdit->textCursor();
        textCursor.movePosition(QTextCursor::Start);
        textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, line - 1);
        ui->textEdit->setTextCursor(textCursor);
        updateLineNumbers();
    }
}

void MainWindow::findAndReplace() {
    if (ui->textEdit->textCursor().hasSelection()) {
        findDialog->setSearchExpression(ui->textEdit->textCursor().selectedText());
    }

    findDialog->show();
    findDialog->activateWindow();
    findDialog->raise();
}

void MainWindow::applyColor() {
    QColor color = QColorDialog::getColor(QColor(0xffcc00), this, tr("Apply Color"));

    if (color.isValid()) {
        if (ui->textEdit->textCursor().hasSelection()) {
            const QString selectionWithColor = "|cff" + color.name().mid(1) + ui->textEdit->textCursor().selection().toPlainText() + "|r";
            ui->textEdit->textCursor().removeSelectedText();
            ui->textEdit->insertPlainText(selectionWithColor);
        } else {
            ui->textEdit->insertPlainText("|cff" + color.name().mid(1) + "MY_TEXT" + "|r");
        }
    }
}

void MainWindow::openCommonj() {
    if (closeFile()) {
        const QString filePath = "wc3reforged/common.j";
        QFile f(filePath);

        if (f.open(QIODevice::ReadOnly)) {
            ui->textEdit->document()->setPlainText(f.readAll());
            resetDocumentChanges();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file %1").arg(filePath));
        }
    }
}

void MainWindow::openCommonai() {
    if (closeFile()) {
        const QString filePath = "wc3reforged/common.ai";
        QFile f(filePath);

        if (f.open(QIODevice::ReadOnly)) {
            ui->textEdit->document()->setPlainText(f.readAll());
            resetDocumentChanges();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file %1").arg(filePath));
        }
    }
}

void MainWindow::openBlizzardj() {
    if (closeFile()) {
        const QString filePath = "wc3reforged/Blizzard.j";
        QFile f(filePath);

        if (f.open(QIODevice::ReadOnly)) {
            ui->textEdit->document()->setPlainText(f.readAll());
            resetDocumentChanges();
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Could not open file %1").arg(filePath));
        }
    }
}

void MainWindow::openScript() {
    if (closeFile()) {
        QAction *action = dynamic_cast<QAction*>(sender());

        if (action != nullptr) {
            const QString filePath = action->data().toString();
            QFile f(filePath);

            if (f.open(QIODevice::ReadOnly)) {
                ui->textEdit->document()->setPlainText(f.readAll());
                resetDocumentChanges();
            } else {
                QMessageBox::warning(this, tr("Error"), tr("Could not open file %1").arg(filePath));
            }
        }
    }
}

void MainWindow::updateScriptsActions() {
    int index = ui->menuScripts->actions().indexOf(ui->actionBlizzardj);

    if (index != -1) {
        while (ui->menuScripts->actions().size() > index + 1) {
            ui->menuScripts->removeAction(ui->menuScripts->actions().last());
        }
    }

    // add actions for all other script files in the folder
    QFileInfo fileInfo("wc3reforged");

    if (fileInfo.isDir()) {
        bool first = true;

        for (QDirIterator it(fileInfo.fileName(), QStringList() << "*.j", QDir::Files, QDirIterator::Subdirectories); it.hasNext(); ) {
            QString file = it.next();

            if (!file.toLower().endsWith("common.j") && !file.toLower().endsWith("common.ai") && !file.toLower().endsWith("blizzard.j")) {
                if (first) {
                    ui->menuScripts->addSeparator();
                }

                QFileInfo fileInfoScript(file);
                QAction *action = ui->menuScripts->addAction(fileInfoScript.fileName());
                action->setData(file);

                connect(action, &QAction::triggered, this, &MainWindow::openScript);

                first = false;
            }
        }
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

void MainWindow::highlightTokensAndAst(const HighLightInfo & /* highLightInfo */, bool /* checkSyntax */) {
    // TODO Only highlight syntax errors.
    //QList<QTextEdit::ExtraSelection> extraSelections = highLightInfo.toExtraSelections(ui->textEdit->document(), checkSyntax);

    //qDebug() << "Extra selections" << extraSelections.size();

    //ui->textEdit->setExtraSelections(extraSelections);
}

void MainWindow::astListItemDoubleClicked(QListWidgetItem *item) {
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
    //const int visibleLines = ui->textEdit->height() / ui->textEdit->fontMetrics().height();

    qDebug() << "Visible lines" << visibleLines << "starting at" << startLine;

    QList<qreal> lineHeights;

    for (int i = 0; i < visibleLines; i++) {
        const qreal lineHeight = ui->textEdit->document()->documentLayout()->blockBoundingRect(startCursor.block()).height(); // TODO is 0 before anything happens
        lineHeights.push_back(lineHeight); // since it can be set a min height
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

void MainWindow::openJASSManual() {
    QDesktopServices::openUrl(QUrl("http://jass.sourceforge.net/doc/"));
}

void MainWindow::openCodeOnHive() {
    QDesktopServices::openUrl(QUrl("https://www.hiveworkshop.com/forums/code.718/"));
}

void MainWindow::openPJassUpdates() {
    QDesktopServices::openUrl(QUrl("https://www.hiveworkshop.com/threads/pjass-updates.258738/"));
}

void MainWindow::aboutPJassDialog() {
    PJass pjass;
    int pjassExitCode = pjass.runVersion();
    QString version = pjass.getVersion();

    QMessageBox::about(this, tr("pjass %1").arg(version), tr("JASS syntax checker."));
}

void MainWindow::aboutDialog() {
    QMessageBox::about(this, tr("Baradé's vJass IDE %1").arg(VJASSIDE_VERSION), tr("Integrated development environment for the scripting languages JASS and vJass from the computer game Warcraft III. This is an Open Source application based on the Qt framework. Visit <a href=\"https://github.com/tdauth/vjasside\">the GitHub repository</a> to contribute or star it."));
}

void MainWindow::restartTimer() {
    if (timerId != 0) {
        killTimer(timerId);
    }

    qDebug() << "Restarting user input timer";
    // wait 2 seconds after the user has stopped writing something
    timerId = startTimer(2000);

    // TODO Restart the thread, so it will wait for the next user input instead of continuing with the current scanning/parsing/highlighting.

    updateWindowStatusBar();
}

void MainWindow::documentChanges() {
    documentHasChanged = true;
    syncDocumentState = false;
    updateWindowTitle();
    updateLineNumbers();
}

void MainWindow::resetDocumentChanges() {
    documentHasChanged = false;
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

    updateCurrentLineHighLighting();

    updateWindowStatusBar();
}

void MainWindow::updateWindowStatusBar() {
    const int currentColumn = ui->textEdit->textCursor().positionInBlock();
    const int currentLine = ui->textEdit->textCursor().blockNumber();
    const HighLightInfo::Location location = HighLightInfo::Location(currentLine, currentColumn);

    //qDebug() << "Size of AST elements by location" << astElementyByLocation.size();
    //qDebug() << "Location line" << location.line << "and column" << location.column;

    const QString parserState = timerId != 0 ? tr("Waiting for user stopping") : (syncDocumentState ? tr("Done") : tr("Parsing"));

    if (currentResults != nullptr && currentResults->highLightInfo.getAstElementsByLocation().contains(location)) {
        VJassAst *ast = currentResults->highLightInfo.getAstElementsByLocation()[location];

        statusBar()->showMessage(tr("Column: %1, Line: %2      Parser State: %3      Parser: %4      %5").arg(currentColumn + 1).arg(currentLine + 1).arg(parserState).arg(parserName).arg(ast->toString()));
    } else {
        statusBar()->showMessage(tr("Column: %1, Line: %2      Parser State: %3      Parser: %4").arg(currentColumn + 1).arg(currentLine + 1).arg(parserState).arg(parserName));
    }
}

void MainWindow::pauseParserThread() {
    scanAndParsePaused.storeRelease(1);
}

void MainWindow::resumeParserThread() {
    scanAndParsePaused.storeRelease(0);
}

void MainWindow::updatePJassSyntaxCheckerVJassIDE(bool checked) {
    if (checked) {
        syntaxChecker.storeRelease(0);
        parserName ="vjasside";
    } else {
        syntaxChecker.storeRelease(1);
        parserName ="pjass";
    }

    QSignalBlocker signalBlocker(ui->actionPJassSyntaxChecker);
    ui->actionPJassSyntaxChecker->setChecked(!checked);

    // this will update the syntax check
    restartTimer();
}

void MainWindow::updatePJassSyntaxCheckerPJass(bool checked) {
    if (checked) {
        syntaxChecker.storeRelease(1);
        parserName ="pjass";
    } else {
        syntaxChecker.storeRelease(0);
        parserName ="vjasside";
    }

    QSignalBlocker signalBlocker(ui->actionVJassIDESyntaxChecker);
    ui->actionVJassIDESyntaxChecker->setChecked(!checked);

    // this will update the syntax check
    restartTimer();
}

void MainWindow::setAnalyzeMemoryLeaks(bool checked) {
    analyzeMemoryLeaks.storeRelease(checked);

    QSignalBlocker signalBlocker(ui->actionAnalyzeMemoryLeaks);
    ui->actionAnalyzeMemoryLeaks->setChecked(checked);

    // this will update the memory analysis
    restartTimer();
}

namespace {

inline void setTextBlockBackgroundColor(QTextCursor textCursor, int lineStart, int lineEnd, QColor backgroundColor) {
    QTextBlockFormat textBlockFormat;

    textCursor.setPosition(0);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineStart);
    textCursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, lineEnd - lineStart);
    //textCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    textBlockFormat.setBackground(backgroundColor);
    textCursor.setBlockFormat(textBlockFormat);
}

inline void rehighlightBlocks(SyntaxHighlighter *syntaxHighlighter, QTextDocument *textDocument, int lineStart, int lineEnd) {
    for (int i = lineStart; i <= lineEnd; i++) {
        syntaxHighlighter->rehighlightBlock(textDocument->findBlockByLineNumber(i));
    }
}

}

void MainWindow::updateCurrentLineHighLighting() {
    QSignalBlocker signalBlocker(ui->textEdit);

    const int previousLineStart = currentLineStart;
    const int previousLineEnd = currentLineEnd;

    currentLineStart = ui->textEdit->textCursor().hasSelection() ? ui->textEdit->document()->findBlock(ui->textEdit->textCursor().selectionStart()).blockNumber() : ui->textEdit->textCursor().blockNumber();
    currentLineEnd = ui->textEdit->textCursor().hasSelection() ? ui->textEdit->document()->findBlock(ui->textEdit->textCursor().selectionEnd()).blockNumber() : ui->textEdit->textCursor().blockNumber();

    QTextCursor textCursor = ui->textEdit->textCursor();
    const int originalPosition = textCursor.position();

    setTextBlockBackgroundColor(textCursor, previousLineStart, previousLineEnd, QColor(0xffffff));
    setTextBlockBackgroundColor(textCursor, currentLineStart, currentLineEnd, QColor(0xfaf5d4));

    syntaxHighlighter->setCurrentLineStart(currentLineStart);
    syntaxHighlighter->setCurrentLineEnd(currentLineEnd);

    const QChar characterAfterCursor = ui->textEdit->document()->characterAt(originalPosition);
    int bracketsColumnNumber = -1;

    if (characterAfterCursor == '(' || characterAfterCursor == '[' || characterAfterCursor == ']' || characterAfterCursor == ')') {
        bracketsColumnNumber = ui->textEdit->textCursor().columnNumber();
    } else if (ui->textEdit->textCursor().columnNumber() > 0) {
        const QChar characterBeforeCursor = ui->textEdit->document()->characterAt(originalPosition - 1);

        if (characterBeforeCursor == '(' || characterBeforeCursor == '[' || characterBeforeCursor == ']' || characterBeforeCursor == ')') {
            bracketsColumnNumber = ui->textEdit->textCursor().columnNumber() - 1;
        }
    }

    if (bracketsColumnNumber != -1) {
        syntaxHighlighter->setHighlightBracketPosition(ui->textEdit->textCursor().blockNumber(), bracketsColumnNumber);
    } else {
        syntaxHighlighter->setHighlightBracketPosition(-1, -1);
    }

    rehighlightBlocks(syntaxHighlighter, ui->textEdit->document(), previousLineStart, previousLineEnd);
    rehighlightBlocks(syntaxHighlighter, ui->textEdit->document(), currentLineStart, currentLineEnd);

    textCursor.setPosition(originalPosition);

    //qDebug() << "Selection line start" << currentLineStart << "and end" << currentLineEnd;
}

void MainWindow::clearAllHighLighting() {
    QSignalBlocker signalBlocker(ui->textEdit);
    QTextCursor cursor(ui->textEdit->document());
    QTextCharFormat fmtNormal;
    fmtNormal.setBackground(Qt::white);
    fmtNormal.setFontWeight(QFont::Normal);
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.setCharFormat(fmtNormal);
}

void MainWindow::updateOutliner() {
    ui->outlinerListWidget->clear();

    if (currentResults != nullptr) {
        for (const VJassAst *astElement : currentResults->highLightInfo.getAstElements()) {
            const QString text = astElement->toString();

            if ((text.startsWith(VJassToken::KEYWORD_NATIVE) && ui->checkBoxNatives->isChecked())
                || (text.startsWith(VJassToken::KEYWORD_FUNCTION) && ui->checkBoxFunctions->isChecked())
                || (text.startsWith(VJassToken::KEYWORD_CONSTANT) && ui->checkBoxConstants->isChecked())
                || (text.startsWith(VJassToken::KEYWORD_TYPE) && ui->checkBoxTypes->isChecked())
                || (!text.startsWith(VJassToken::KEYWORD_NATIVE) && !text.startsWith(VJassToken::KEYWORD_FUNCTION) && !text.startsWith(VJassToken::KEYWORD_CONSTANT) && !text.startsWith(VJassToken::KEYWORD_TYPE) && ui->checkBoxGlobals->isChecked())
            ) {
                QListWidgetItem *item = new QListWidgetItem(tr("%1 - line %2 and column %3").arg(astElement->toString()).arg(astElement->getLine() + 1).arg(astElement->getColumn() + 1));
                item->setData(Qt::UserRole, QPoint(astElement->getLine(), astElement->getColumn()));
                ui->outlinerListWidget->addItem(item);
            }
        }
    }

    if (ui->outlinerListWidget->count() == 0) {
        ui->outlinerListWidget->addItem(tr("No matching elements."));
        ui->tabWidget->setTabText(1, tr("0 Elements"));
    } else {
        ui->tabWidget->setTabText(1, tr("%n Elements", "%n Elements", currentResults->highLightInfo.getAstElements().length()));
    }
}

void MainWindow::updateMemoryLeaks() {
    ui->memoryLeaksListWidget->clear();

    if (currentResults != nullptr) {
        for (const VJassAst *astElement : currentResults->highLightInfo.getAstLeakingElements()) {
            const QString text = astElement->toString();

            QListWidgetItem *item = new QListWidgetItem(tr("%1 - line %2 and column %3").arg(astElement->toString()).arg(astElement->getLine() + 1).arg(astElement->getColumn() + 1));
            item->setData(Qt::UserRole, QPoint(astElement->getLine(), astElement->getColumn()));
            ui->memoryLeaksListWidget->addItem(item);
        }
    }

    if (ui->memoryLeaksListWidget->count() == 0) {
        ui->memoryLeaksListWidget->addItem(tr("No memory leaks."));
        ui->tabWidget->setTabText(2, tr("0 Memory Leaks"));
    } else {
        ui->tabWidget->setTabText(2, tr("%n Memory Leaks", "%n Memory Leaks", currentResults->highLightInfo.getAstLeakingElements().length()));
    }
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

        updateWindowStatusBar();
    } else if (timerId == 0 && event->timerId() == timerIdCheck) {
        //qDebug() << "Checking scan and parse result";

        ScanAndParseResults *scanAndParseResults = this->scanAndParseResults.fetchAndStoreAcquire(nullptr);

        // if there are results we will highlight everything now
        if (scanAndParseResults != nullptr) {

            if (currentResults != nullptr) {
                delete currentResults;
                currentResults = nullptr;
            }

            currentResults = scanAndParseResults;
            syncDocumentState = true;
            updateWindowStatusBar();

            qDebug() << "Got scan and parse result from thread into the main window";

            bool checkSyntax = ui->actionEnableSyntaxCheck->isChecked();
            bool autoComplete = expectAutoComplete;
            expectAutoComplete = false; // reset
            bool highlight = ui->actionEnableSyntaxHighlighting->isChecked();

            if (checkSyntax || autoComplete || highlight) {
                if (highlight) {
                    qDebug() << "Highlight!";
                    highlightTokensAndAst(currentResults->highLightInfo, checkSyntax);
                }

                if (checkSyntax || autoComplete) {
                    // update syntax errors output widget
                    ui->outputListWidget->clear();

                    const QList<VJassParseError> &parseErrors = currentResults->highLightInfo.getParseErrors();

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

                    // update outliner
                    updateOutliner();

                    // update memory leaks
                    updateMemoryLeaks();

                    if (autoComplete && currentResults->ast->getCodeCompletionSuggestions().size() > 0) {
                        popup->clear();
                        popup->setFocusProxy(this);
                        QTreeWidgetItem *firstItem = nullptr;

                        for (VJassAst *codeCompletionSuggestion : currentResults->ast->getCodeCompletionSuggestions()) {
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
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    updateLineNumbers();

    QMainWindow::resizeEvent(event);
}
