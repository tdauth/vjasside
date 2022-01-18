#include <QtCore>

#include "vjassparser.h"
#include "vjassast.h"
#include "vjassfunction.h"
#include "vjasskeyword.h"

VJassParser::VJassParser()
{
}

inline bool isValidType(const VJassAst &ast, const QString &type) {
    QSet<QString> standardTypes;
    // TODO parse natives instead of adding them here
    standardTypes.insert("integer");
    standardTypes.insert("real");
    standardTypes.insert("boolean");
    standardTypes.insert("unit");

    return standardTypes.contains(type);
}

inline bool isValidIdentifier(const QString &identifier) {
    QSet<QString> keywords;
    keywords.insert("function");
    keywords.insert("takes");
    keywords.insert("nothing");
    keywords.insert("returns");

    return QRegularExpression("[a-zA-Z0-9]+").match(identifier).hasMatch() && !keywords.contains(identifier);
}

VJassAst VJassParser::parse(QString content) {
    VJassAst ast(0, 0);
    int l = 1;
    int c = 1;

    for (const QString &line : content.split("\n")) {
        const QString normalizedLine = line.trimmed();
        // TODO tokenize by keeping the column positions and add the comments (block comments and final line comments) to them.
        // We need the columns and lines to highlight the document.
        const QList<QString> tokens = normalizedLine.split(QRegularExpression("\\s+")); // TODO is not empty if the line is empty

        if (!normalizedLine.isEmpty() && tokens.size() > 0) {
            const QString normalizedToken = tokens.at(0).trimmed();

            // function
            if (normalizedLine.startsWith("function")) {
                VJassFunction *vjassFunction = new VJassFunction(l, c);

                if (tokens.size() == 1) {
                    c = line.indexOf("function") + QString("function").length();

                    vjassFunction->addError(l, c, "Missing function declaration identifier.");
                } else {
                    QString identifier = tokens.at(1);

                    if (isValidIdentifier(identifier)) {
                        vjassFunction->setIdentifier(identifier);

                        if (tokens.size() == 2) {
                            c = line.indexOf(identifier) + QString(identifier).length();

                            vjassFunction->addError(l, c, "Missing takes keyword.");
                        } else {
                            QString takesKeyword = tokens.at(2);

                            if (takesKeyword != "takes") {
                                c = line.indexOf(takesKeyword);

                                vjassFunction->addError(l, c, "Expected takes keyword instead of " + takesKeyword);
                            } else {
                                if (tokens.size() == 3) {
                                    c = line.indexOf(takesKeyword) + QString(takesKeyword).length();

                                    vjassFunction->addError(l, c, "Missing parameters.");
                                    // function parameters
                                } else {
                                    bool gotError = false;
                                    bool gotSeparator = false;
                                    bool expectMoreParameters = false;
                                    int parameterCounter = 0;
                                    int index = 3;

                                    for (; index < tokens.size() && !gotError; ) {
                                        expectMoreParameters = false;

                                        const QString &parameterType = tokens.at(index);

                                        if  (parameterType == "nothing") {
                                            if (parameterCounter > 0) {
                                                c = line.indexOf(parameterType) + QString(parameterType).length();

                                                vjassFunction->addError(l, c, "Unexpected nothing keyword");
                                            }

                                            break;
                                        } else if (index == tokens.size() - 1) {
                                            c = line.indexOf(parameterType) + QString(parameterType).length();

                                            vjassFunction->addError(l, c, "Missing parameter name");
                                            gotError = true;
                                        } else {
                                            if (!isValidType(ast, parameterType)) {
                                                c = line.indexOf(parameterType) + QString(parameterType).length();

                                                vjassFunction->addError(l, c, "Invalid parameter type: " + parameterType);
                                            }


                                            QString parameterName = tokens.at(index + 1);

                                            gotSeparator = parameterName.endsWith(",");

                                            if (gotSeparator) {
                                                expectMoreParameters = true;
                                                parameterName = parameterName.left(parameterName.length() - 1);
                                            }

                                            if (isValidIdentifier(parameterName)) {
                                                vjassFunction->addParameter(parameterType, parameterName);
                                            } else {
                                                c = line.indexOf(parameterName) + parameterName.length();

                                                vjassFunction->addError(l, c, "Invalid parameter name: " + parameterName);
                                                gotError = true;
                                            }
                                        }

                                        index += 2;
                                        parameterCounter++;


                                        if (!gotSeparator) {
                                            if (index < tokens.size()) {
                                                int tmpIndex = index + 1;
                                                const QString &separatorToken = tokens.at(tmpIndex);

                                                if (separatorToken == ",") {
                                                    expectMoreParameters = true;
                                                    index = tmpIndex;
                                                } else if (separatorToken != "returns") {
                                                    c = line.indexOf(separatorToken);

                                                    vjassFunction->addError(l, c, "Expected , but got  " + separatorToken);
                                                    gotError = true;
                                                }


                                            }
                                        }
                                    }

                                    if (!gotError) {
                                        if (expectMoreParameters) {
                                            // TODO got column of last token

                                            vjassFunction->addError(l, c, "Expected more parameters!");
                                        } else {
                                            index++;

                                            if (tokens.size() <= index) {
                                                c = line.length();

                                                vjassFunction->addError(l, c, "Missing returns!");
                                            } else {
                                                const QString &returnsToken = tokens.at(index);

                                                if (returnsToken != "returns") {
                                                    c = line.indexOf(returnsToken);

                                                    vjassFunction->addError(l, c, "Expected returns instead of " + returnsToken);
                                                } else {
                                                    index++;

                                                    if (tokens.size() == index) {
                                                        c = line.length();

                                                        vjassFunction->addError(l, c, "Missing return type!");
                                                    } else {
                                                        const QString &returnType = tokens.at(index);

                                                        vjassFunction->setReturnType(returnType);

                                                        index++;

                                                        if (tokens.size() > index) {
                                                            const QString startingToken = tokens.at(index);

                                                            if (!startingToken.startsWith("//") && !startingToken.startsWith("/*")) {
                                                                c = line.indexOf(startingToken);

                                                                vjassFunction->addError(l, c, "Expected comment instead of " + startingToken);
                                                            } else {
                                                                // TODO use the whole line from column of starting token.
                                                                vjassFunction->addComment(startingToken);

                                                                for ( ; index < tokens.size(); index++) {
                                                                    vjassFunction->addComment(tokens.at(index));
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                }
                            }
                        }

                    } else {
                        c = line.indexOf(identifier) + identifier.length();

                        vjassFunction->addError(l, c, "Invalid identifier: " + identifier);
                    }
                }

                ast.addChild(vjassFunction);
            }
            // unknown token
            else {
                ast.addError(l, c, "Unknown token " + line);
            }
        } else {
            VJassKeyword *functionKeyword = new VJassKeyword(l, c);
            functionKeyword->setKeyword("function");
            ast.addCodeCompletionSuggestion(functionKeyword);

            VJassKeyword *globalsKeyword = new VJassKeyword(l, c);
            globalsKeyword->setKeyword("globals");
            ast.addCodeCompletionSuggestion(globalsKeyword);

            VJassKeyword *constantKeyword = new VJassKeyword(l, c);
            constantKeyword->setKeyword("constant");
            ast.addCodeCompletionSuggestion(constantKeyword);
        }

        l++;
    }


    return ast;
}
