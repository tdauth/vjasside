#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QModelIndex>
#include <QListWidgetItem>
#include <QThread>
#include <QAtomicInt>
#include <QAtomicPointer>

#include "vjassparser.h"
#include "autocompletionpopup.h"

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
    void quit();

    void updateSyntaxErrors(bool checkSyntax, bool autoComplete, bool highlight);
    void updateSyntaxErrorsOnly();
    void updateSyntaxErrorsWithAutoComplete();

    void updateLineNumbersView();
    void showWhiteSpaces();
    void updateLineNumbers();

    void clickPopupItem(const QModelIndex &index);

    void aboutDialog();

    void restartTimer();
    void documentChanges();

    void updateWindowTitle();

    void updateSelectedLines();

private slots:
    void highlightTokens(const QList<VJassToken> &tokens);
    void highlightAst(VJassAst *ast);
    void outputListItemDoubleClicked(QListWidgetItem *item);

    void clearAllHighLighting();

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;

    bool documentHasChanged = false;
    QString fileDir;

    bool expectAutoComplete = false;
    AutoCompletionPopup *popup;

    // the scanner and parser is executed in a separate thread
    // it waits for some text input to be scanned and parsed and returns it for the main window to be retrieved
    // there is a timer which waits for some time until the user doesnt input anything anymore
    // only when this timer is not running anymore we retrieve the scan and parse results every X seconds
    // as soon as they are available we update the text edit
    struct ScanAndParseResults {
        QList<VJassToken> tokens;
        // TODO keep on stack but without heap there happens weird stuff on copying!
        VJassAst *ast;

        ScanAndParseResults(QList<VJassToken> &&tokens, VJassAst *ast) : tokens(std::move(tokens)), ast(ast) {
        }

        virtual ~ScanAndParseResults() {
            delete ast;
            ast = nullptr;
        }
    };

    int timerId;
    int timerIdCheck;
    QAtomicPointer<QString> scanAndParseInput;
    QAtomicPointer<ScanAndParseResults> scanAndParseResults;
    QThread *scanAndParseThread;
};
#endif // MAINWINDOW_H
