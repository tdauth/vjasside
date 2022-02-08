#include <QtCore>

#include "vjassscanner.h"

VJassScanner::VJassScanner()
{

}

inline bool isNotFollowedByIdentifier(const QString &currentContent, int length) {
    // TODO Space and operators cannot follow but not text
    return length <= currentContent.length() || !QRegularExpression("[A-Za-z_0-9]{1}").match(currentContent.mid(length, 1)).hasMatch();
}

QList<VJassToken> VJassScanner::scan(const QString &content, bool dropWhiteSpaces) {
    QList<VJassToken> result;
    int line = 0;
    int column = 0;

    // TODO Generated lexers are much faster probably since they have internal state machines going from one symbol to the next.
    // TODO Use a Symbol table instead of storing all the identifiers for tokens since we often have the same symbol more than once.
    for (int i = 0; i < content.size(); ) {
        // TODO Avoid copying at all cost -> some string view, should not by copied by implicit sharing.
        QString currentContent = content.mid(i);

        bool matched = false;

        // boolean literal true
        if (currentContent.startsWith(VJassToken::KEYWORD_TRUE) && isNotFollowedByIdentifier(currentContent, VJassToken::KEYWORD_TRUE.length())) {
            const int length = VJassToken::KEYWORD_TRUE.length();
            result.push_back(VJassToken(VJassToken::KEYWORD_TRUE, line, column, VJassToken::TrueKeyword));
            column += length;
            i += length;
            matched = true;
        // boolean literal false
        } else if (currentContent.startsWith(VJassToken::KEYWORD_FALSE) && isNotFollowedByIdentifier(currentContent, VJassToken::KEYWORD_FALSE.length())) {
            const int length = VJassToken::KEYWORD_FALSE.length();
            result.push_back(VJassToken(VJassToken::KEYWORD_FALSE, line, column, VJassToken::FalseKeyword));
            column += length;
            i += length;
            matched = true;
        }

        if (!matched) {
            for (const QString &keyword : VJassToken::KEYWRODS_ALL) { // TODO Except TRUE and FALSE
                if (currentContent.startsWith(keyword) && isNotFollowedByIdentifier(currentContent, keyword.length())) {
                    result.push_back(VJassToken(keyword, line, column, VJassToken::typeFromKeyword(keyword)));
                    i += keyword.length();
                    column += keyword.length();

                    //qDebug() << "Matched keyword" << keyword << " with length" << keyword.length();

                    matched = true;

                    break;
                }
            }
        }

        if (!matched) {
            if (currentContent.startsWith("\n")) {
                result.push_back(VJassToken("\n", line, column, VJassToken::LineBreak));
                i += 1;
                line += 1;
                column = 0;
            } else if (currentContent.startsWith(" ") || currentContent.startsWith("\t")) {
                int j = i + 1;

                for ( ; j < content.length(); ++j) {
                    if (content.mid(j, 1) != " " && content.mid(j, 1) != "\t") {
                        break;
                    }
                }

                const int length = j - i;
                //qDebug() << "Whitespaces with length " << length;

                if (!dropWhiteSpaces) {
                    result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::WhiteSpace));
                }

                i += length;
                column += length;
            } else if (currentContent.startsWith(",")) {
                result.push_back(VJassToken(",", line, column, VJassToken::Separator));
                i += 1;
                column += 1;
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

                const int length = j - i;

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

                const int length = j - i;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::Comment));

                column += columns;
                lines += lines;
                i += length;
            // Comparison Operator
            } else if (currentContent.startsWith("<") || currentContent.startsWith(">") || currentContent.startsWith("==") || currentContent.startsWith("<=") || currentContent.startsWith(">=") || currentContent.startsWith("!=")) {
                const int length = 2;
                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::ComparisonOperator));
                column += length;
                i += length;
            // Assignment Operator
            } else if (currentContent.startsWith("=")) {
                result.push_back(VJassToken(content.mid(i, 1), line, column, VJassToken::AssignmentOperator));

                column += 1;
                i += 1;
            // operator
            } else if (currentContent.startsWith("/") || currentContent.startsWith("+") || currentContent.startsWith("-") || currentContent.startsWith("*")) {
                result.push_back(VJassToken(content.mid(i, 1), line, column, VJassToken::Operator));

                column += 1;
                i += 1;
            // real literal
            } else if (currentContent.startsWith(".")) {
                int j = i + 1;

                for ( ; j < content.size(); j++) {
                    if (!QRegularExpression("[0-9]{1}").match(QString(content.at(j))).hasMatch()) {
                        break;
                    }
                }

                const int length = j - i;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::RealLiteral));

                column += length;
                i += length;

            //decimal         := [1-9][0-9]*
            //octal           := '0'[0-7]*
            //hex             := '$'[0-9a-fA-F]+ | '0'[xX][0-9a-fA-F]+
            // fourcc          := ''' .{4} '''
            // integer or real literal
            } else if (QRegularExpression("\\$[0-9a-fA-F]{1}").match(QString(content.mid(i, 2))).hasMatch()) {
                int j = i + 2;

                for ( ; j < content.size(); j++) {
                    if (!QRegularExpression("[0-9a-fA-F]{1}").match(QString(content.at(j))).hasMatch()) {
                        break;
                    }
                }

                const int length = j - i;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::IntegerLiteral));

                column += length;
                i += length;
            } else if (QRegularExpression("[0-9]{1}").match(QString(content.at(i))).hasMatch()) {
                int j = i + 1;
                int numberOfDots = 0;

                for ( ; j < content.size() && numberOfDots <= 1; j++) {
                    if (content.at(j) == '.') {
                        numberOfDots++;
                    } else if (!QRegularExpression("[0-9]{1}").match(QString(content.at(j))).hasMatch()) {
                        break;
                    }
                }

                const int length = j - i;

                if (numberOfDots == 0) {
                    result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::IntegerLiteral));
                } else {
                    result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::RealLiteral));
                }

                column += length;
                i += length;
            // rawcode literal
            } else if (currentContent.startsWith("'")) {
                int j = i + 1;

                for ( ; j < content.size(); j++) {
                    if (!QRegularExpression("[A-Za-z0-9]{1}").match(QString(content.at(j))).hasMatch() || content.at(j) == '\'') {
                        break;
                    }
                }

                const int length = j - i;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::RawCodeLiteral));

                column += length;
                i += length;
            // string literal
            } else if (currentContent.startsWith("\"")) {
                int j = i + 1;

                for ( ; j < content.size(); j++) {
                    if (content.at(j) == '\"') {
                        break;
                    }
                }

                const int length = j - i + 1; // consume the second double quotes as well

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::StringLiteral));

                column += length;
                i += length;
            // EscapeLiteral not inside of a string
            } else if (currentContent.startsWith("\\")) {
                const int length = 1;

                result.push_back(VJassToken(content.mid(i, length), line, column, VJassToken::EscapeLiteral));

                column += length;
                i += length;
            // left bracket
            } else if (currentContent.startsWith("(")) {
                result.push_back(VJassToken(content.mid(i, 1), line, column, VJassToken::LeftBracket));

                column += 1;
                i += 1;
            // right bracket
            } else if (currentContent.startsWith(")")) {
                result.push_back(VJassToken(content.mid(i, 1), line, column, VJassToken::RightBracket));

                column += 1;
                i += 1;
            // left square bracket
            } else if (currentContent.startsWith("[")) {
                result.push_back(VJassToken(content.mid(i, 1), line, column, VJassToken::LeftSquareBracket));

                column += 1;
                i += 1;
            // right square bracket
            } else if (currentContent.startsWith("]")) {
                result.push_back(VJassToken(content.mid(i, 1), line, column, VJassToken::RightSquareBracket));

                column += 1;
                i += 1;
            // text
            // TODO match a whole group maybe
            } else if (QRegularExpression("[A-Za-z_]{1}").match(QString(content.at(i))).hasMatch()) {
                int j = i + 1;

                for ( ; j < content.size(); j++) {
                    if (!QRegularExpression("[A-Za-z0-9_]{1}").match(QString(content.at(j))).hasMatch()) {
                        break;
                    }
                }

                const int length = j - i;

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
