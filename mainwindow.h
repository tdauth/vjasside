#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QModelIndex>

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
    void saveAs();

    void updateSyntaxErrors(bool autoComplete);
    void updateSyntaxErrorsOnly();
    void updateSyntaxErrorsWithAutoComplete();

    void clickPopupItem(const QModelIndex &index);

    void aboutDialog();

private:
    Ui::MainWindow *ui;

    VJassParser vjassParser;

    AutoCompletionPopup *popup;
};
#endif // MAINWINDOW_H
