#include <QtCore>

#include "vjassnative.h"
#include "vjassfunction.h"
#include "vjassglobal.h"
#include "vjasstype.h"
#include "pjass.h"
#include "jasshelper.h"
#include "memoryleakanalyzer.h"
#include "highlightinfo.h"

HighLightInfo::HighLightInfo(const QString &text, const QList<VJassToken> &tokens, VJassAst *ast, const QList<VJassParseError> &parseErrors, bool fillCustomTextCharFormat, bool createTextDocument, bool analyzeMemoryLeaks) : ast(ast), textDocument(nullptr), parseErrors(parseErrors)
{
    //qDebug() << "Getting tokens" << tokens.size();

    // filter for elements which need to be highlighted
    if (fillCustomTextCharFormat) {
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
                } else if (token.getType() == VJassToken::TrueKeyword || token.getType() == VJassToken::FalseKeyword) {
                    customTextCharFormat.applyForegroundColor = true;
                    customTextCharFormat.foregroundColor = Qt::blue;
                } else if (token.getType() == VJassToken::RawCodeLiteral || token.getType() == VJassToken::IntegerLiteral || token.getType() == VJassToken::RealLiteral) {
                    customTextCharFormat.applyForegroundColor = true;
                    customTextCharFormat.foregroundColor = Qt::darkYellow;
                } else if (token.getType() == VJassToken::StringLiteral) {
                    customTextCharFormat.applyForegroundColor = true;
                    customTextCharFormat.foregroundColor = Qt::red;
                    // TODO highlight escape sequence inside of the string
                } else if (token.getType() == VJassToken::EscapeLiteral) {
                    customTextCharFormat.applyForegroundColor = true;
                    customTextCharFormat.foregroundColor = QColor(0xFFC0CB);
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
                    } else if (token.isBlizzardJConstant()) {
                        //qDebug() << "BlizzardJ COnstant";
                        customTextCharFormat.applyForegroundColor = true;
                        customTextCharFormat.foregroundColor = QColor(0x00008b);
                        customTextCharFormat.isItalic = true;
                    } else if (token.isBlizzardJGlobal()) {
                        customTextCharFormat.applyForegroundColor = true;
                        customTextCharFormat.foregroundColor = QColor(0x497c8b);
                        customTextCharFormat.isItalic = true;
                    } else if (token.isBlizzardJFunction()) {
                        customTextCharFormat.applyForegroundColor = true;
                        customTextCharFormat.foregroundColor = QColor(0xff0000);
                        customTextCharFormat.isBold = true;
                        // color="#6b8e23" selColor="#ffffff" bold="0" italic="1
                    } else if (token.isCommonAIConstant()) {
                        customTextCharFormat.applyForegroundColor = true;
                        customTextCharFormat.foregroundColor = QColor(0x6b8e23);
                        customTextCharFormat.isItalic = true;
                    } else if (token.isCommonAIGlobal()) {
                        customTextCharFormat.applyForegroundColor = true;
                        customTextCharFormat.foregroundColor = QColor(0x5c8e6c);
                        customTextCharFormat.isItalic = true;
                    } else if (token.isCommonAINative()) {
                        customTextCharFormat.applyForegroundColor = true;
                        customTextCharFormat.foregroundColor = QColor(0x218B21);
                        customTextCharFormat.isBold = true;
                    } else if (token.isCommonAIFunction()) {
                        customTextCharFormat.applyForegroundColor = true;
                        customTextCharFormat.foregroundColor = QColor(0x00CD63);
                        customTextCharFormat.isBold = true;
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


        // store all AST elements for the outliner
        QStack<VJassAst*> stack;
        stack.push_back(ast);

        while (!stack.isEmpty()) {
            VJassAst *a = stack.pop();

            if (typeid(*a) == typeid(VJassNative)
                || typeid(*a) == typeid(VJassFunction)
                || typeid(*a) == typeid(VJassGlobal)
                || typeid(*a) == typeid(VJassType)
                ) {
                astElements.push_back(a);
            }

            astElementsByLocation.insert(Location(a->getLine(), a->getColumn()), a);

            for (VJassAst *child : a->getChildren()) {
                stack.push_back(child);
            }
        }

        VJassAst::sortByPosition(astElements);
        std::sort(this->parseErrors.begin(), this->parseErrors.end(), [](VJassParseError &e1, VJassParseError &e2) {
           const int lineDiff = e1.getLine() - e2.getLine();

           if (lineDiff == 0) {
               return e1.getColumn() - e2.getColumn();
           }

           return lineDiff;
        });

        if (analyzeMemoryLeaks) {
            MemoryLeakAnalyzer memoryLeakAnalyzer(ast);
            for (VJassGlobal *global : memoryLeakAnalyzer.getGlobals()) {
                astLeakingElements.append(global);
            }
        }
    }

    if (createTextDocument) {
        textDocument = new QTextDocument(text);
        textDocument->setDocumentLayout(new QPlainTextDocumentLayout(textDocument));
        textDocument->setDefaultFont(getNormalFont());
        textDocument->setIndentWidth(20.0);
        //textDocument->setDefaultTextOption(QTextOption::)

        // This is the slow method creating all the extra selections!
        // TODO This is the really slow part because of the iteration with the cursors. It might take about 30 seconds or longer.
        // TODO If we use QSyntaxHighlighter properly, we can get rid of this stuff?
        qDebug() << "Beginning highlighting code elements with elements size:" << getFormattedLocations().size();
        QElapsedTimer timer;
        timer.start();

        // The text document has signals such as "cursorPositionChanged" etc. we do not need to be emitted here.
        QSignalBlocker signalBlockerTextDocument(textDocument);

        for (auto iterator = customTextCharFormats.constKeyValueBegin(); iterator != customTextCharFormats.constKeyValueEnd(); ++iterator) {
            const Location &location = iterator->first;
            const CustomTextCharFormat &customTextCharFormat = iterator->second;

            // move a cursor to the character
            //QTextCursor cursor(textDocument);
            QTextCursor cursor(textDocument);
            //QSignalBlocker signalBlockerTextCursor(&cursor);
            cursor.setPosition(0, QTextCursor::MoveAnchor);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, location.line);
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, location.column);

            // format all characters
            // TODO This might be the slow part.
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, customTextCharFormat.length);
            QTextCharFormat fmt = getNormalFormat();
            customTextCharFormat.applyToTextCharFormat(fmt, ast != nullptr);
            cursor.setCharFormat(fmt); // for bold it is required that the cursor has the format

            QTextEdit::ExtraSelection extraSelection;
            extraSelection.cursor = cursor;
            extraSelection.format = fmt;
            extraSelections.push_back(extraSelection);
        }

        qDebug() << "Ending highlighting code elements with elements size:" << getFormattedLocations().size() << "and elapsed time" << timer.elapsed() << "ms and in seconds" << (timer.elapsed() / 1000) << "and in minutes" << (timer.elapsed() / (1000 * 60));
    }
}

HighLightInfo::~HighLightInfo() {
    if (ast != nullptr) {
        delete ast;
        ast = nullptr;
    }
}

void HighLightInfo::CustomTextCharFormat::applyToTextCharFormat(QTextCharFormat &fmt, bool checkSyntax) const {
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

QFont HighLightInfo::getNormalFont() {
    QFont font;
    font.setFamily("Courier");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    font.setPointSize(10);

    return font;
}

void HighLightInfo::applyNormalFormat(QTextCharFormat &textCharFormat) {
    textCharFormat.setBackground(Qt::white);
    textCharFormat.setForeground(Qt::black);
    textCharFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
    textCharFormat.setFontItalic(false);
    textCharFormat.setFontWeight(QFont::Normal);
    textCharFormat.setFont(getNormalFont());
}

QTextCharFormat HighLightInfo::getNormalFormat() {
    // reset formatting for upcoming text
    QTextCharFormat fmtNormal;
    applyNormalFormat(fmtNormal);

    return fmtNormal;
}

const QMap<HighLightInfo::Location, HighLightInfo::CustomTextCharFormat>& HighLightInfo::getFormattedLocations() const {
    return customTextCharFormats;
}

QList<QTextEdit::ExtraSelection> HighLightInfo::toExtraSelections(QTextDocument * /* textDocument */, bool /* checkSyntax */) const {
    // TODO use checkSyntax to remove all the underlining etc.
    return extraSelections;
}

VJassAst* HighLightInfo::getAst() const {
    return ast;
}

QTextDocument* HighLightInfo::getTextDocument() const {
    return textDocument;
}

const QList<VJassParseError>& HighLightInfo::getParseErrors() const {
    return parseErrors;
}

const QList<VJassAst*>& HighLightInfo::getAstElements() const {
    return astElements;
}

const QMap<HighLightInfo::Location, VJassAst*>& HighLightInfo::getAstElementsByLocation() const {
    return astElementsByLocation;
}

const QList<VJassAst*>& HighLightInfo::getAstLeakingElements() const {
    return astLeakingElements;
}

HighLightInfo::CustomTextCharFormat& HighLightInfo::getCustomTextCharFormat(int line, int column) {
    const Location key = Location(line, column);

    if (!customTextCharFormats.contains(key)) {
        customTextCharFormats.insert(key, CustomTextCharFormat());
    }

    return customTextCharFormats[key];
}
