#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QModelIndex>
#include <QListWidgetItem>
#include <QLabel>
#include <QThread>
#include <QAtomicInt>
#include <QAtomicPointer>

#include "vjassparser.h"
#include "syntaxhighlighter.h"
#include "autocompletionpopup.h"
#include "highlightinfo.h"
#include "finddialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void newFile();
    void openFile();
    bool saveAs();
    bool closeFile();
    void quit();

    void goToLine();
    void findAndReplace();
    void applyColor();

    void openCommonj();
    void openCommonai();
    void openBlizzardj();
    void openScript();

    void updateSyntaxErrors(bool checkSyntax, bool autoComplete, bool highlight);
    void updateSyntaxErrorsOnly();
    void updateSyntaxErrorsWithAutoComplete();

    void updateLineNumbersView();
    void showWhiteSpaces();
    void updateLineNumbers();

    void clickPopupItem(const QModelIndex &index);

    void openJASSManual();
    void openCodeOnHive();
    void openPJassUpdates();

    void aboutPJassDialog();
    void aboutJassHelperDialog();
    void aboutDialog();

    void restartTimer();
    void documentChanges();

    void resetDocumentChanges();
    void updateWindowTitle();

    void updateSelectedLines();

    void updateWindowStatusBar();

    void pauseParserThread();
    void resumeParserThread();

private slots:
    void updateScriptsActions();

    void updateCursorPosition(int position);
    void highlightTokensAndAst(const HighLightInfo &highLightInfo, bool checkSyntax);
    void astListItemDoubleClicked(QListWidgetItem *item);

    void updatePJassSyntaxCheckerVJassIDE(bool checked);
    void updatePJassSyntaxCheckerPJass(bool checked);
    void updateSyntaxCheckerJassHelper(bool checked);

    void updateSyntaxCheckerFromActions();

    void setAnalyzeMemoryLeaks(bool checked);

    void updateCurrentLineHighLighting();

    void clearAllHighLighting();

    void updateOutliner();
    void updateMemoryLeaks();

    friend class TestMainWindow;
    friend class SyntaxHighLighter;

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;

    // we use a permanent status bar widget
    QLabel *statusBar = nullptr;

    FindDialog *findDialog = nullptr;

    // selection
    int currentLineStart = 0;
    int currentLineEnd = 0;

    bool documentHasChanged = false;
    QString fileDir;

    SyntaxHighlighter *syntaxHighlighter = nullptr;

    bool expectAutoComplete = false;
    AutoCompletionPopup *popup;

    // the scanner and parser is executed in a separate thread
    // it waits for some text input to be scanned and parsed and returns it for the main window to be retrieved
    // there is a timer which waits for some time until the user doesnt input anything anymore
    // only when this timer is not running anymore we retrieve the scan and parse results every X seconds
    // as soon as they are available we update the text edit
    int timerId;
    int timerIdCheck;
    QAtomicPointer<QString> scanAndParseInput;
    QAtomicPointer<HighLightInfo> scanAndParseResults;
    QAtomicInt scanAndParsePaused;
    QThread *scanAndParseThread;
    QAtomicInt stopScanAndParseThread;
    QAtomicInt syntaxChecker; // 0 - vjasside, 1 - pjass
    QAtomicInt analyzeMemoryLeaks; // 0 - on, 1 - off
    QString parserName;
    bool syncDocumentState = true;

    HighLightInfo *currentResults = nullptr;
};
#endif // MAINWINDOW_H
