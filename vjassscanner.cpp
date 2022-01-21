#include <QtCore>

#include "vjassscanner.h"

VJassScanner::VJassScanner()
{

}

QList<VJassToken> VJassScanner::scan(const QString &content, bool dropWhiteSpaces) {
    QList<VJassToken> result;
    int line = 0;
    int column = 0;

    for (int i = 0; i < content.size(); i++) {
        QString currentContent = content.mid(i);

        bool matched = false;

        for (const QString &keyword : VJassToken::KEYWRODS_ALL) {
            if (currentContent.startsWith(keyword)) {
                result.push_back(VJassToken(keyword, line, column, VJassToken::typeFromKeyword(keyword)));
                i += keyword.length();
                column += keyword.length();

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
                int j = 0;

                for (j = 0;  j < content.length(); ++j) {
                    if (content.mid(j, 1) != " " && content.mid(j, 1) != "\t") {
                        break;
                    }
                }

                QString whiteSpaces = content.mid(0, j);

                if (!dropWhiteSpaces) {
                    result.push_back(VJassToken(whiteSpaces, line, column, VJassToken::WhiteSpace));
                }

                i += whiteSpaces.length();
                column += whiteSpaces.length();
            } else if (currentContent == ",") {
                result.push_back(VJassToken(",", line, column, VJassToken::Separator));
                i += 1;
                column += 1;

                matched = true;
            // line comment
            } else if (currentContent == "/" && i + 1 < content.size() && content.at(i + 1) == '/') {
                int j = i;

                for ( ; j < content.size(); j++) {
                    if (content.at(j) == '\n') {
                        break;
                    }
                }

                result.push_back(VJassToken(content.mid(i, j - i), line, column, VJassToken::Comment));

                column += j - i;
                i = j - 1;
            // block comment
            }  else if (currentContent == "/" && i + 1 < content.size() && content.at(i + 1) == '*') {
                int j = i;
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

                result.push_back(VJassToken(content.mid(i, j - i), line, column, VJassToken::Comment));

                column += columns;
                lines += lines;
                i = j - 1;
            // operator
            } else if (currentContent == "/" || currentContent == "+" || currentContent == "-" || currentContent == "*") {
                result.push_back(VJassToken(currentContent, line, column, VJassToken::Operator));

                column += 1;
                i += 1;
            // text
            } else if (QRegularExpression("[A-Za-z0-9]{1}").match(QString(content.at(i))).hasMatch()) {
                int j = i;

                for ( ; j < content.size(); j++) {
                    if (!QRegularExpression("[A-Za-z0-9]{1}").match(QString(content.at(j))).hasMatch()) {
                        break;
                    }
                }

                result.push_back(VJassToken(content.mid(i, j - i), line, column, VJassToken::Text));

                column += j - i;
                i = j - 1;
            } else {
                result.push_back(VJassToken(currentContent, line, column, VJassToken::Unknown));

                column += 1;
                i += 1;
            }
        }
    }

    return result;
}
