#include <QtGui>
#include <QtWidgets>

#include "syntaxhighlighter.h"
#include "vjassscanner.h"
#include "highlightinfo.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    //qDebug() << "Highlight block" << text;
    qDebug() << "High light block by syntax highlighter";

    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(text, true);
    HighLightInfo highLightInfo(text, tokens, nullptr, "", "", true, false);

    const QMap<HighLightInfo::Location, HighLightInfo::CustomTextCharFormat> &formattedLocations = highLightInfo.getFormattedLocations();

    for (auto iterator = formattedLocations.constKeyValueBegin(); iterator != formattedLocations.constKeyValueEnd(); ++iterator) {
        const HighLightInfo::Location &location = iterator->first;
        const HighLightInfo::CustomTextCharFormat &customTextCharFormat = iterator->second;
        QTextCharFormat fmt = highLightInfo.getNormalFormat();
        customTextCharFormat.applyToTextCharFormat(fmt, false);
        fmt.setBackground(QBrush()); // do not apply any background color
        setFormat(location.column, customTextCharFormat.length, fmt);
    }

    emit updatedHighlighting();
}
