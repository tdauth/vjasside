#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent);

    void setCurrentLineStart(int currentLineStart);
    void setCurrentLineEnd(int currentLineEnd);

protected:
    virtual void highlightBlock(const QString &text) override;

private:
    int currentLineStart;
    int currentLineEnd;
};

#endif // SYNTAXHIGHLIGHTER_H
