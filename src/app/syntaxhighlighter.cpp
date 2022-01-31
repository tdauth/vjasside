#include <QtGui>
#include <QtWidgets>

#include "syntaxhighlighter.h"
#include "vjassscanner.h"
#include "highlightinfo.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent), currentLine(0) {
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

    if (currentLine == currentBlock().blockNumber()) {
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

        if (currentLine == currentBlock().blockNumber()) {
            fmt.setBackground(QColor(0xfaf5d4));
        } else {
            fmt.setBackground(QColor(0xffffff));
        }

        setFormat(location.column, customTextCharFormat.length, fmt);
    }
}

void SyntaxHighlighter::setCurrentLine(int currentLine) {
    this->currentLine = currentLine;
}
