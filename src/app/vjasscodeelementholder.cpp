#include <QtCore>

#include "vjasscodeelementholder.h"

VJassCodeElementHolder::VJassCodeElementHolder(const QList<VJassToken> &tokens, VJassAst *ast)
{
    qDebug() << "Getting tokens" << tokens.size();

    // filter for elements which need to be highlighted
    for (const VJassToken &token : tokens) {
        if (token.highlight()) {
            CustomTextCharFormat &customTextCharFormat = getCustomTextCharFormat(token.getLine(), token.getColumn());
            customTextCharFormat.length = token.getLength();

            // formats are taken from https://github.com/tdauth/syntaxhighlightings/blob/master/Kate/vjass.xml
            // TODO You should be able to configure them in the settings but this has no high priority right now.
            if (token.isValidKeyword()) {
                customTextCharFormat.applyForegroundColor = true;
                customTextCharFormat.foregroundColor = Qt::black;
                customTextCharFormat.isBold = true;

                //qDebug() << "Is keyword";
            } else if (token.getType() == VJassToken::Comment) {
                customTextCharFormat.applyForegroundColor = true;
                customTextCharFormat.foregroundColor = Qt::gray;
                customTextCharFormat.isItalic = true;
                //qDebug() << "Is comment";
            } else if (token.getType() == VJassToken::BooleanLiteral) {
                customTextCharFormat.applyForegroundColor = true;
                customTextCharFormat.foregroundColor = Qt::blue;
            } else if (token.getType() == VJassToken::RawCodeLiteral || token.getType() == VJassToken::IntegerLiteral || token.getType() == VJassToken::RealLiteral) {
                customTextCharFormat.applyForegroundColor = true;
                customTextCharFormat.foregroundColor = Qt::darkYellow;
            } else if (token.getType() == VJassToken::Text) {
                // Make a quick check for the symbol from hash sets of standard types and functions so we have these highlighted even without syntax checking
                if (token.isCommonJType()) {
                    customTextCharFormat.applyForegroundColor = true;
                    customTextCharFormat.foregroundColor = Qt::blue;
                } else if (token.isCommonJNative()) {
                    customTextCharFormat.applyForegroundColor = true;
                    customTextCharFormat.foregroundColor = QColor(0xba55d3);
                    customTextCharFormat.isBold = true;
                } else if (token.isCommonJConstant()) {
                    customTextCharFormat.applyForegroundColor = true;
                    customTextCharFormat.foregroundColor = QColor(0xff7f50);
                    customTextCharFormat.isItalic = true;
                } else {
                    qDebug() << "Token type should get some highlighting config:" << token.getValue();

                    Q_ASSERT(false);
                }
            } else {
                qDebug() << "Token type should get some highlighting config:" << token.getValue();

                Q_ASSERT(false);
            }
        }
    }

    if (ast != nullptr) {
        // storing the errors prevents invalid memory references
        /*
         TODO format everything existing with parse errors but keep the same formats
        const QList<VJassParseError> &allParseErrors = ast->getAllParseErrors();

        for (const VJassParseError &parseError : allParseErrors) {
            // TODO Handle multiple lines.
            for (int column = parseError.getColumn(); column < parseError.getColumn() + parseError.getLength(); column++) {
                //qDebug() << "Text char format of parse error in line " << parseError.getLine() << "and column" << column;

                CustomTextCharFormat &customTextCharFormat = getCustomTextCharFormat(parseError.getLine(), column);
                customTextCharFormat.syntaxError = true;
            }
        }
        */
    }
}

void VJassCodeElementHolder::CustomTextCharFormat::applyToTextCharFormat(QTextCharFormat &fmt, bool checkSyntax) const {
    //qDebug() << "Applying custom format in line" << line << "and column" << column;

    if (syntaxError && checkSyntax) {
        //qDebug() << "Syntax error!";
        fmt.setUnderlineColor(Qt::red);
        fmt.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    }

    if (isItalic) {
        //qDebug() << "Italic!";
        fmt.setFontItalic(true);
    }

    if (isBold) {
        //qDebug() << "Bold!";
        fmt.setFontWeight(QFont::Bold);
    }

    if (applyForegroundColor) {
        //qDebug() << "Foreground!";
        fmt.setForeground(foregroundColor);
    }
}

const QMap<VJassCodeElementHolder::Location, VJassCodeElementHolder::CustomTextCharFormat>& VJassCodeElementHolder::getFormattedLocations() const {
    return customTextCharFormats;
}

VJassCodeElementHolder::CustomTextCharFormat& VJassCodeElementHolder::getCustomTextCharFormat(int line, int column) {
    const Location key = Location(line, column);

    if (!customTextCharFormats.contains(key)) {
        customTextCharFormats.insert(key, CustomTextCharFormat());
    }

    return customTextCharFormats[key];
}
