#include <QtCore>

#include "vjassscanner.h"

VJassScanner::VJassScanner()
{

}

QList<VJassToken> VJassScanner::scan(const QString &content, bool dropWhiteSpaces) {
    QList<VJassToken> result;
    int line = 0;
    int column = 0;

    for (int i = 0; i < content.size(); ) {
        // TODO Avoid copying at all cost -> some string view
        QString currentContent = content.mid(i);

        bool matched = false;

        for (const QString &keyword : VJassToken::KEYWRODS_ALL) {
            if (currentContent.startsWith(keyword)) {
                result.push_back(VJassToken(keyword, line, column, VJassToken::typeFromKeyword(keyword)));
                i += keyword.length();
                column += keyword.length();

                //qDebug() << "Matched keyword" << keyword << " with length" << keyword.length();

                matched = true;

                break;
            }
        }

        if (!matched) {
            if (currentContent.startsWith("\n")) {
                result.push_back(VJassToken("\n", line, column, VJassToken::LineBreak));
                i += 1;
                line += 1;
                column = 0;

                matched = true;
            } else if (currentContent.startsWith(" ") || currentContent.startsWith("\t")) {
                int j = i + 1;

                for ( ; j < content.length(); ++j) {
                    if (content.mid(j, 1) != " " && content.mid(j, 1) != "\t") {
                        break;
                    }
                }

                int length = j - i;
                //qDebug() << "Whitespaces with length " << length;

                if (!dropWhiteSpaces) {
                    result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::WhiteSpace));
                }

                i += length;
                column += length;

                matched = true;
            } else if (currentContent.startsWith(",")) {
                result.push_back(VJassToken(",", line, column, VJassToken::Separator));
                i += 1;
                column += 1;

                matched = true;
            // line comment
            } else if (currentContent.startsWith("//")) {
                int j = i + 2;
                bool gotLineBreakEnd = false;

                for ( ; j < content.size() && !gotLineBreakEnd; ) {
                    if (content.at(j) == '\n') {
                        gotLineBreakEnd = true;
                    } else {
                        j++;
                    }
                }

                int length = j - i;

                //qDebug() << "Line comment with length" << length;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::Comment));

                column += length;
                i += length;
            // block comment
            }  else if (currentContent.startsWith("/*")) {
                int j = i + 2;
                int columns = 0;
                int lines = 0;

                for ( ; j < content.size(); j++) {
                    if (content.at(j) == '\n') {
                        lines++;
                        columns = 0;
                    } else if (content.at(j) == '*' && j + 1 < content.size() && content.at(j + 1) == '/') {
                        j++;
                        columns++;

                        break;
                    } else {
                        columns++;
                    }
                }

                int length = j - i;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::Comment));

                column += columns;
                lines += lines;
                i += length;
            // operator
            } else if (currentContent.startsWith("/") || currentContent.startsWith("+") || currentContent.startsWith("-") || currentContent.startsWith("*") || currentContent.startsWith("=")) {
                result.push_back(VJassToken(currentContent.mid(i, 1), line, column, VJassToken::Operator));

                column += 1;
                i += 1;
            // boolean literal true
            } else if (currentContent.startsWith("true")) {
                result.push_back(VJassToken(currentContent.mid(i, 4), line, column, VJassToken::BooleanLiteral));

                column += 4;
                i += 4;
            // boolean literal false
            } else if (currentContent.startsWith("false")) {
                result.push_back(VJassToken(currentContent.mid(i, 5), line, column, VJassToken::BooleanLiteral));

                column += 5;
                i += 5;
            // text
            // TODO match a whole group maybe
            } else if (QRegularExpression("[A-Za-z0-9]{1}").match(QString(content.at(i))).hasMatch()) {
                int j = i + 1;

                for ( ; j < content.size(); j++) {
                    if (!QRegularExpression("[A-Za-z0-9]{1}").match(QString(content.at(j))).hasMatch()) {
                        break;
                    }
                }

                int length = j - i;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::Text));


                column += length;
                i += length;
            } else {
                result.push_back(VJassToken(content.mid(i, 1), line, column, VJassToken::Unknown));

                column += 1;
                i += 1;
            }
        }
    }

    return result;
}
