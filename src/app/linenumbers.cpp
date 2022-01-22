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
