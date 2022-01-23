#include <QtGui>
#include <QtWidgets>

#include "linenumbers.h"
#include "ui_linenumbers.h"

LineNumbers::LineNumbers(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LineNumbers)
{
    ui->setupUi(this);
}

LineNumbers::~LineNumbers() {
    delete ui;
    ui = nullptr;
}

void LineNumbers::setLineNumbers(int start, int lineNumbers, const QList<qreal> &lineHeights) {
    ui->listWidget->clear();

    qDebug() << "Set line numbers from" << start << "with count" << lineNumbers;

    for (int i = 0; i < lineNumbers; i++) {
        const int lineNumber = start + i;
        QListWidgetItem *listWidgetItem = new QListWidgetItem(ui->listWidget);
        listWidgetItem->setText(QString::number(lineNumber + 1));
        listWidgetItem->setData(Qt::UserRole, lineNumber);
        //QFont font = listWidgetItem->font();
        //font.setPointSizeF(lineHeights.at(i));
        //listWidgetItem->setFont(font);
        listWidgetItem->setSizeHint(QSize(30, lineHeights.at(i)));

        qDebug() << "Setting line height of line" << i << "to" << lineHeights.at(i);
        //listWidgetItem->setSizeHint(QSize(10, lineHeights.at(i)));
    }
}

void LineNumbers::updateSelectedLines(int lineStart, int lineEnd) {
    qDebug() << "Set selected line numbers from" << lineStart << "to" << lineEnd;

    for (int i = 0; i < ui->listWidget->count(); i++) {
        const int lineNumber = ui->listWidget->item(i)->data(Qt::UserRole).toInt();
        QFont font = ui->listWidget->item(i)->font();

        if (lineNumber >= lineStart && lineNumber <= lineEnd) {
            font.setBold(true);
            ui->listWidget->item(i)->setFont(font);
        } else {
            font.setBold(false);
            ui->listWidget->item(i)->setFont(font);
        }
    }
}
