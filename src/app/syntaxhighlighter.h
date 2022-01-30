#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent);

signals:
    void updatedHighlighting();

protected:
    virtual void highlightBlock(const QString &text) override;
};

#endif // SYNTAXHIGHLIGHTER_H
