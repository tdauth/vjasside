#include <QtGui>
#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , popup(new AutoCompletionPopup)
{
    ui->setupUi(this);

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrorsOnly);
    connect(ui->actionComplete, &QAction::triggered, this, &MainWindow::updateSyntaxErrorsWithAutoComplete);
    connect(ui->actionBaradesVJassIDE, &QAction::triggered, this, &MainWindow::aboutDialog);

    connect(popup, &QTreeWidget::clicked, this, &MainWindow::clickPopupItem);

    updateSyntaxErrorsOnly();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete popup;
}

void MainWindow::saveAs() {
    QString saveFileName = QFileDialog::getSaveFileName(this);

    if (saveFileName.size() > 0) {
        QFileDialog::saveFileContent(ui->textEdit->toPlainText().toUtf8());
    }
}

void MainWindow::updateSyntaxErrors(bool autoComplete) {
    VJassAst ast = this->vjassParser.parse(ui->textEdit->toPlainText());
    QString browserOutput;
    int number = 0;

    for (VJassParseError &parseError : ast.getAllParseErrors()) {
        if (number > 0) {
            browserOutput += "\n";
        }

        browserOutput += tr("Syntax error at line %1 and column %2: %3").arg(parseError.getLine() + 1).arg(parseError.getColumn() + 1).arg(parseError.getError());
        number++;
    }

    if (number > 0) {
        ui->textBrowser->setText(browserOutput);
    } else {
        ui->textBrowser->setText("No syntax errors.");
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

void MainWindow::updateSyntaxErrorsOnly() {
    updateSyntaxErrors(false);
}

void MainWindow::updateSyntaxErrorsWithAutoComplete() {
    updateSyntaxErrors(true);
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
