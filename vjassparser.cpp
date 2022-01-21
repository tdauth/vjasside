#include <QtCore>

#include "vjassparser.h"
#include "vjassscanner.h"
#include "vjassast.h"
#include "vjassfunction.h"
#include "vjasskeyword.h"

VJassParser::VJassParser()
{
}

VJassAst VJassParser::parse(QString content) {
    VJassScanner scanner;

    VJassAst ast(0, 0);
    QList<VJassToken> tokens = scanner.scan(content, true);
    bool isInFunction = false;

    for (int i = 0; i < tokens.size(); i++) {
        const VJassToken &token = tokens.at(i);
        bool gotNextLine = false;

        // function
        if (token.getType() == VJassToken::FunctionKeyword) {
            isInFunction = true;
            VJassFunction *vjassFunction = new VJassFunction(token.getLine(), token.getColumn());
            i++;

            if (i == tokens.size()) {
                vjassFunction->addError(token.getLine(), token.getColumn() + token.getValue().length(), "Missing function declaration identifier.");
            } else {
                const VJassToken &identifier = tokens.at(i);

                if (identifier.isValidIdentifier()) {
                    vjassFunction->setIdentifier(identifier.getValue());
                    i++;

                    if (i == tokens.size()) {
                        vjassFunction->addError(identifier, "Missing takes keyword.");
                        VJassKeyword *takesKeyword = new VJassKeyword(identifier.getLine(), identifier.getColumn());
                        takesKeyword->setKeyword(VJassToken::KEYWORD_TAKES);
                        ast.addCodeCompletionSuggestion(takesKeyword);
                    } else {
                        const VJassToken &takesKeyword = tokens.at(i);
                        i++;

                        if (takesKeyword.getType() != VJassToken::TakesKeyword) {
                            vjassFunction->addError(takesKeyword, "Expected takes keyword instead of " + takesKeyword.getValue());
                        } else {
                            if (i == tokens.size()) {
                                vjassFunction->addErrorAtEndOf(takesKeyword, "Missing parameters.");
                            // function parameters
                            } else {
                                bool gotError = false;
                                bool expectMoreParameters = false;
                                int parameterCounter = 0;

                                for (i = i - 1; i < tokens.size() && !gotError; ) {
                                    expectMoreParameters = false;

                                    const VJassToken &parameterType = tokens.at(i);

                                    if  (parameterType.getType() == VJassToken::NothingKeyword) {
                                        if (parameterCounter > 0) {
                                            vjassFunction->addErrorAtEndOf(parameterType, "Unexpected nothing keyword");
                                        }

                                        break;
                                    } else if (i == tokens.size() - 1) {
                                        vjassFunction->addErrorAtEndOf(parameterType, "Missing parameter name");
                                        gotError = true;
                                    } else {
                                        if (!parameterType.isValidType()) {
                                            vjassFunction->addErrorAtEndOf(parameterType, "Invalid parameter type: " + parameterType.getValue());
                                        }


                                        const VJassToken &parameterName = tokens.at(i + 1);

                                        if (parameterName.isValidIdentifier()) {
                                            vjassFunction->addParameter(parameterType.getValue(), parameterName.getValue());
                                        } else {
                                            vjassFunction->addErrorAtEndOf(parameterName, "Invalid parameter name: " + parameterName.getValue());
                                            gotError = true;
                                        }
                                    }

                                    i += 2;
                                    parameterCounter++;

                                    if (i < tokens.size()) {
                                        int tmpIndex = i + 1;
                                        const VJassToken &separatorToken = tokens.at(tmpIndex);

                                        if (separatorToken.getType() == VJassToken::Separator) {
                                            expectMoreParameters = true;
                                            i = tmpIndex;
                                        } else if (separatorToken.getType() != VJassToken::ReturnsKeyword) {
                                            vjassFunction->addError(separatorToken, "Expected , but got  " + separatorToken.getValue());
                                            gotError = true;
                                        }
                                    }
                                }

                                if (!gotError) {
                                    if (expectMoreParameters) {
                                        vjassFunction->addErrorAtEndOf(tokens.at(i), "Expected more parameters!");
                                    } else {
                                        i++;

                                        if (tokens.size() <= i) {
                                            vjassFunction->addErrorAtEndOf(tokens.at(i - 1), "Missing returns!");
                                        } else {
                                            const VJassToken &returnsToken = tokens.at(i);

                                            if (returnsToken.getType() != VJassToken::ReturnsKeyword) {
                                                vjassFunction->addError(returnsToken, "Expected returns instead of " + returnsToken.getValue());
                                            } else {
                                                i++;

                                                if (tokens.size() == i) {
                                                    vjassFunction->addErrorAtEndOf(returnsToken, "Missing return type!");
                                                } else {
                                                    const VJassToken &returnType = tokens.at(i);

                                                    vjassFunction->setReturnType(returnType.getValue());

                                                    i++;
                                                }
                                            }
                                        }
                                    }
                                }

                            }
                        }
                    }

                } else {
                    vjassFunction->addError(identifier, "Invalid identifier: " + identifier.getValue());
                }
            }

            ast.addChild(vjassFunction);
        } else if (token.getType() == VJassToken::EndfunctionKeyword) {
            if (isInFunction) {
                isInFunction = true;
                ast.addError(token, "Unknown symbol: " + token.getValue());
            } else {
                ast.addError(token, "Unexpected symbol: " + token.getValue());
            }
        } else if (token.getType() == VJassToken::Unknown) {
            ast.addError(token, "Unknown symbol: " + token.getValue());
        } else if (token.getType() == VJassToken::Text) {
            if (!isInFunction) {
                if (VJassToken::KEYWORD_FUNCTION.startsWith(token.getValue())) {
                    VJassKeyword *functionKeyword = new VJassKeyword(token.getLine(), token.getColumn() + token.getValue().size());
                    functionKeyword->setKeyword(VJassToken::KEYWORD_FUNCTION);
                    ast.addCodeCompletionSuggestion(functionKeyword);
                }
            } else {
                if (VJassToken::KEYWORD_ENDFUNCTION.startsWith(token.getValue())) {
                    VJassKeyword *endfunctionKeyword = new VJassKeyword(token.getLine(), token.getColumn() + token.getValue().size());
                    endfunctionKeyword->setKeyword(VJassToken::KEYWORD_ENDFUNCTION);
                    ast.addCodeCompletionSuggestion(endfunctionKeyword);
                }
            }
        }

        // add comments
        if (tokens.size() > i) {
            const VJassToken &startingToken = tokens.at(i);
            VJassAst *child = ast.getChildren().isEmpty() ? &ast : ast.getChildren().last();

            if (startingToken.getType() != VJassToken::Comment && startingToken.getType() != VJassToken::LineBreak) {
                child->addError(startingToken, "Expected comment instead of " + startingToken.getValue());
            } else if (startingToken.getType() == VJassToken::Comment) {
                // TODO use the whole line from column of starting token.
                child->addComment(startingToken.getValue());
            } else {
                gotNextLine = true;
            }
        }

        // until next line
        if (!gotNextLine) {
            for ( ; i < tokens.size(); i++) {
                 const VJassToken startingToken = tokens.at(i);

                 if (startingToken.getType() == VJassToken::LineBreak) {
                     i++;

                     break;
                 }
            }
        }
    }

    if (tokens.isEmpty()) {
        VJassKeyword *functionKeyword = new VJassKeyword(0, 0);
        functionKeyword->setKeyword(VJassToken::KEYWORD_FUNCTION);
        ast.addCodeCompletionSuggestion(functionKeyword);

        VJassKeyword *globalsKeyword = new VJassKeyword(0, 0);
        globalsKeyword->setKeyword(VJassToken::KEYWORD_GLOBALS);
        ast.addCodeCompletionSuggestion(globalsKeyword);

        VJassKeyword *constantKeyword = new VJassKeyword(0, 0);
        constantKeyword->setKeyword(VJassToken::KEYWORD_CONSTANT);
        ast.addCodeCompletionSuggestion(constantKeyword);

        VJassKeyword *commentLineKeyword = new VJassKeyword(0, 0);
        commentLineKeyword->setKeyword("//");
        ast.addCodeCompletionSuggestion(commentLineKeyword);

        VJassKeyword *commentBlockKeyword = new VJassKeyword(0, 0);
        commentBlockKeyword->setKeyword("/*");
        ast.addCodeCompletionSuggestion(commentBlockKeyword);
    }

    return ast;
}
