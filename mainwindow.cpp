#include <QtGui>
#include <QtWidgets>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , popup(new QTreeWidget)
{
    ui->setupUi(this);

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::updateSyntaxErrors);
    connect(ui->actionComplete, &QAction::triggered, this, &MainWindow::updateSyntaxErrors);

    connect(popup, &QTreeWidget::clicked, this, &MainWindow::clickPopupItem);

    updateSyntaxErrors();
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

    if (number > 0) {
        ui->textBrowser->setText(browserOutput);
    } else {
        ui->textBrowser->setText("No syntax errors.");
    }

    if (ast.getCodeCompletionSuggestions().size() > 0) {
        popup->clear();
        popup->headerItem()->setText(0, tr("Auto Completion"));
        popup->setMinimumSize(QSize(128, 128));
        popup->setWindowFlags(Qt::Popup);
        popup->setFocusPolicy(Qt::NoFocus);
        popup->setFocusProxy(this);

        for (VJassAst *codeCompletionSuggestion : ast.getCodeCompletionSuggestions()) {
            new QTreeWidgetItem(popup, QStringList(codeCompletionSuggestion->toString()));
        }

        new QTreeWidgetItem(popup, QStringList(tr("Cancel")));

        popup->move(ui->textEdit->cursor().pos());

        // TODO set at the end of the cursor

        popup->show();
    }
}

void MainWindow::clickPopupItem(const QModelIndex &index) {
    if (index.row() == popup->topLevelItemCount() - 1) {
        popup->close();
    } else {
        ui->textEdit->insertPlainText(index.data().toString());
    }

    popup->close();
}

