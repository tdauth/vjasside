#include <QtGui>
#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->textEdit, QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrors);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveAs() {
    QString saveFileName = QFileDialog::getSaveFileName(this);

    if (saveFileName.size() > 0) {
        QFileDialog::saveFileContent(ui->textEdit->toPlainText().toUtf8());
    }
}

void MainWindow::updateSyntaxErrors() {
    VJassAst ast = this->vjassParser.parse(ui->textEdit->toPlainText());
    QString browserOutput;
    int number = 0;

    for (VJassParseError &parseError : ast.getAllParseErrors()) {
        if (number > 0) {
            browserOutput += "\n";
        }

        browserOutput += tr("Syntax error at line %1 and column %2: %3").arg(parseError.getLine()).arg(parseError.getColumn()).arg(parseError.getError());
        number++;
    }

    ui->textBrowser->setText(browserOutput);
}

