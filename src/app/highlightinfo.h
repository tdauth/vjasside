#ifndef HIGHLIGHTINFO_H
#define HIGHLIGHTINFO_H

#include <QTextCharFormat>
#include <QMap>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextDocument>

#include "vjasstoken.h"
#include "vjassast.h"

/**
 * @brief The VJassCodeElementHolder class
 *
 * Holds code elements and stores their formatting for each line and column.
 * Stores the number of upcoming characters with the exact same formatting as well.
 *
 * TODO Rename to VJassHighlightInfo or something like that.
 */
class HighLightInfo
{
public:
    HighLightInfo(const QString &text, const QList<VJassToken> &tokens, VJassAst *ast, bool createTextDocument = false);

    struct Location {
        int line;
        int column;

        Location(int line, int column) : line(line), column(column) {
        }

        Location(const Location &other)
            : line(other.line)
            , column(other.column) {
        }

        Location& operator=(const Location &other) {
            this->line = other.line;
            this->column = other.column;

            return *this;
        }
    };

    struct CustomTextCharFormat {
        bool syntaxError;
        bool isBold;
        bool isItalic;
        bool applyForegroundColor;
        QColor foregroundColor;
        // the number of upcoming characters with the exact same format
        int length;

        void applyToTextCharFormat(QTextCharFormat &fmt, bool checkSyntax) const;

        CustomTextCharFormat() : syntaxError(false), isBold(false), isItalic(false), applyForegroundColor(false), length(0) {
        }

        CustomTextCharFormat(const CustomTextCharFormat &other)
            : syntaxError(other.syntaxError)
            , isBold(other.isBold)
            , isItalic(other.isItalic)
            , applyForegroundColor(other.applyForegroundColor)
            , length(other.length) {
        }

        CustomTextCharFormat& operator=(const CustomTextCharFormat &other) {
            this->syntaxError = other.syntaxError;
            this->isBold = other.isBold;
            this->isItalic = other.isItalic;
            this->applyForegroundColor = other.applyForegroundColor;
            this->foregroundColor = other.foregroundColor;
            this->length = other.length;

            return *this;
        }
    };

    const QMap<Location, CustomTextCharFormat>& getFormattedLocations() const;
    QList<QTextEdit::ExtraSelection> toExtraSelections(QTextDocument *textDocument, bool checkSyntax) const;
    QTextDocument* getTextDocument() const;
    const QList<VJassParseError>& getParseErrors() const;
    const QList<VJassAst*>& getAstElements() const;
    const QMap<Location, VJassAst*>& getAstElementsByLocation() const;

    static QFont getNormalFont();
    static void applyNormalFormat(QTextCharFormat &textCharFormat);
    static QTextCharFormat getNormalFormat();

private:
    CustomTextCharFormat& getCustomTextCharFormat(int line, int column);

    QMap<Location, CustomTextCharFormat> customTextCharFormats;
    QTextDocument *textDocument;
    QList<QTextEdit::ExtraSelection> extraSelections;
    QList<VJassParseError> parseErrors;
    QList<VJassAst*> astElements;
    QMap<Location, VJassAst*> astElementsByLocation;
};

inline bool operator<(const HighLightInfo::Location &e1, const HighLightInfo::Location &e2) {
    const int lineDiff = e2.line - e1.line;

    return lineDiff > 0 || (lineDiff == 0 && e2.column > e1.column);
}

inline bool operator==(const HighLightInfo::CustomTextCharFormat &e1, const HighLightInfo::CustomTextCharFormat &e2) {
    return e1.applyForegroundColor == e2.applyForegroundColor
            && e1.foregroundColor == e2.foregroundColor
            && e1.isBold == e2.isBold
            && e1.isItalic == e2.isItalic
            && e1.syntaxError == e2.syntaxError;
}

#endif // HIGHLIGHTINFO_H
