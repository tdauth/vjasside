#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
    SyntaxHighlighter(QTextDocument *parent);
    SyntaxHighlighter(QObject *parent);

protected:
    virtual void highlightBlock(const QString &text) override;
};

#endif // SYNTAXHIGHLIGHTER_H
