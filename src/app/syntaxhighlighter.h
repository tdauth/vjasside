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
    void setHighlightBracketPosition(int line, int column);

protected:
    virtual void highlightBlock(const QString &text) override;

private:
    int currentLineStart;
    int currentLineEnd;
    int highlightBracketLine;
    int highlightBracketColumn;
};

#endif // SYNTAXHIGHLIGHTER_H
