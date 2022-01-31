#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(QTextDocument *parent);

    void setCurrentLine(int currentLine);

protected:
    virtual void highlightBlock(const QString &text) override;

private:
    int currentLine;
};

#endif // SYNTAXHIGHLIGHTER_H
