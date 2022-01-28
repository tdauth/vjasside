#include <QtGui>
#include <QtWidgets>

#include "syntaxhighlighter.h"
#include "vjassscanner.h"
#include "highlightinfo.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
}

SyntaxHighlighter::SyntaxHighlighter(QObject *parent) : QSyntaxHighlighter(parent) {
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    VJassScanner scanner;
    QList<VJassToken> tokens = scanner.scan(text, true);
    HighLightInfo highLightInfo(text, tokens, nullptr, false);

    const QMap<HighLightInfo::Location, HighLightInfo::CustomTextCharFormat> &formattedLocations = highLightInfo.getFormattedLocations();

    for (auto iterator = formattedLocations.constKeyValueBegin(); iterator != formattedLocations.constKeyValueEnd(); ++iterator) {
        const HighLightInfo::Location &location = iterator->first;
        const HighLightInfo::CustomTextCharFormat &customTextCharFormat = iterator->second;
        QTextCharFormat fmt = highLightInfo.getNormalFormat();
        customTextCharFormat.applyToTextCharFormat(fmt, false);
        setFormat(location.column, customTextCharFormat.length, fmt);
    }
}
