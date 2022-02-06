#include <QtGui>
#include <QtWidgets>

#include "syntaxhighlighter.h"
#include "vjassscanner.h"
#include "highlightinfo.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent), currentLineStart(0), currentLineEnd(0), highlightBracketLine(-1), highlightBracketColumn(-1) {
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    //qDebug() << "Highlight block" << text;
    //qDebug() << "Highlight block by syntax highlighter" << currentBlock().blockNumber();

    // only format necessary tokens
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(text, true);
    HighLightInfo highLightInfo(text, tokens, nullptr, "", "", true, false);

    // the background color depends on whether it is the current line
    QTextCharFormat textCharFormat = highLightInfo.getNormalFormat();
    const int currentBlockLine = currentBlock().blockNumber();
    const bool isCurrentLine = currentBlockLine >= currentLineStart && currentBlockLine <= currentLineEnd;

    if (isCurrentLine) {
        textCharFormat.setBackground(QColor(0xfaf5d4));
    } else {
        textCharFormat.setBackground(QColor(0xffffff));
    }

    setFormat(0, text.length(), textCharFormat);

    // highlight all characters which need to be highlighted
    const QMap<HighLightInfo::Location, HighLightInfo::CustomTextCharFormat> &formattedLocations = highLightInfo.getFormattedLocations();

    for (auto iterator = formattedLocations.constKeyValueBegin(); iterator != formattedLocations.constKeyValueEnd(); ++iterator) {
        const HighLightInfo::Location &location = iterator->first;
        const HighLightInfo::CustomTextCharFormat &customTextCharFormat = iterator->second;
        QTextCharFormat fmt = highLightInfo.getNormalFormat();
        customTextCharFormat.applyToTextCharFormat(fmt, false);

        if (isCurrentLine) {
            fmt.setBackground(QColor(0xfaf5d4));
        } else {
            fmt.setBackground(QColor(0xffffff));
        }

        setFormat(location.column, customTextCharFormat.length, fmt);
    }

    if (highlightBracketLine != -1 && highlightBracketColumn != -1 && currentBlock().blockNumber() == highlightBracketLine) {
        QTextCharFormat fmt = format(highlightBracketColumn);
        fmt.setBackground(Qt::green);
        setFormat(highlightBracketColumn, 1, fmt);

        const QString highlightedBracket = text.mid(highlightBracketColumn, 1);

        //qDebug() << "Highlighted bracket" << highlightedBracket;

        if (highlightedBracket == "(" || highlightedBracket == "[") {
            const QString openingBracket = highlightedBracket;
            const QString closingBracket = highlightedBracket == "(" ? ")" : "]";
            int foundOpeningBrackets = 0;
            int foundClosingBrackets = 0;

            for (int i = highlightBracketColumn; i < text.length(); i++) {
                if (text.mid(i, 1) == openingBracket) {
                    foundOpeningBrackets++;
                } else if (text.mid(i, 1) == closingBracket) {
                    foundClosingBrackets++;

                    if (foundClosingBrackets == foundOpeningBrackets) {
                        QTextCharFormat fmt = format(i);
                        fmt.setBackground(Qt::green);
                        setFormat(i, 1, fmt);

                        break;
                    }
                }
            }
        } else if (highlightedBracket == ")" || highlightedBracket == "]") {
            const QString openingBracket = highlightedBracket == ")" ? "(" : "[";
            const QString closingBracket = highlightedBracket;
            int foundOpeningBrackets = 0;
            int foundClosingBrackets = 0;

            for (int i = highlightBracketColumn; i >= 0; i--) {
                if (text.mid(i, 1) == closingBracket) {
                    foundClosingBrackets++;
                } else if (text.mid(i, 1) == openingBracket) {
                    foundOpeningBrackets++;

                    if (foundClosingBrackets == foundOpeningBrackets) {
                        QTextCharFormat fmt = format(i);
                        fmt.setBackground(Qt::green);
                        setFormat(i, 1, fmt);

                        break;
                    }
                }
            }
        }
    }
}

void SyntaxHighlighter::setCurrentLineStart(int currentLineStart) {
    this->currentLineStart = currentLineStart;
}

void SyntaxHighlighter::setCurrentLineEnd(int currentLineEnd) {
    this->currentLineEnd = currentLineEnd;
}

void SyntaxHighlighter::setHighlightBracketPosition(int highlightBracketLine, int highlightBracketColumn) {
    this->highlightBracketLine = highlightBracketLine;
    this->highlightBracketColumn = highlightBracketColumn;
}
