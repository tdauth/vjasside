#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QPlainTextEdit *plainTextEdit, QWidget *parent = nullptr);
    ~FindDialog();

    bool isCaseSensitive() const;
    bool isRegularExpression() const;

public slots:
    void setSearchExpression(const QString &expression);

    bool find(bool next);
    bool findNext();
    bool findPrevious();

    int replace(int startPosition, int maxMatches);
    int replaceNext();
    int replaceAll();

private:
    Ui::FindDialog *ui;
    QPlainTextEdit *plainTextEdit;
};

#endif // FINDDIALOG_H
