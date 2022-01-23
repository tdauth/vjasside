#ifndef VJASSCODEELEMENTHOLDER_H
#define VJASSCODEELEMENTHOLDER_H

#include <QTextCharFormat>
#include <QMap>

#include "vjasscodeelement.h"
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
class VJassCodeElementHolder
{
public:
    VJassCodeElementHolder(const QList<VJassToken> &tokens, VJassAst *ast);

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

private:
    CustomTextCharFormat& getCustomTextCharFormat(int line, int column);

    QMap<Location, CustomTextCharFormat> customTextCharFormats;
};

inline bool operator<(const VJassCodeElementHolder::Location &e1, const VJassCodeElementHolder::Location &e2) {
    const int lineDiff = e2.line - e1.line;

    return lineDiff > 0 || (lineDiff == 0 && e2.column > e1.column);
}

inline bool operator==(const VJassCodeElementHolder::CustomTextCharFormat &e1, const VJassCodeElementHolder::CustomTextCharFormat &e2) {
    return e1.applyForegroundColor == e2.applyForegroundColor
            && e1.foregroundColor == e2.foregroundColor
            && e1.isBold == e2.isBold
            && e1.isItalic == e2.isItalic
            && e1.syntaxError == e2.syntaxError;
}

#endif // VJASSCODEELEMENTHOLDER_H
