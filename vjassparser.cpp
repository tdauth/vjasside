#include <QtCore>

#include "vjassparser.h"
#include "vjassscanner.h"
#include "vjassast.h"
#include "vjassfunction.h"
#include "vjasskeyword.h"
#include "vjasstype.h"

VJassParser::VJassParser()
{
}

VJassAst VJassParser::parse(const QString &content, const QList<VJassToken> &tokens) {
    VJassAst ast(0, 0);
    bool isInFunction = false;

    for (int i = 0; i < tokens.size(); i++) {
        const VJassToken &token = tokens.at(i);
        bool gotNextLine = false;

        // type
        if (token.getType() == VJassToken::TypeKeyword) {
            VJassType *vjassType = new VJassType(token.getLine(), token.getColumn());

            if (isInFunction) {
                vjassType->addError(token, "Cannot declare a type inside of a function.");
                // TODO add code completion suggestion to remove the token
            }

            i++;

            if (i == tokens.size()) {
                vjassType->addErrorAtEndOf(token, "Missing type name.");
                // TODO add code completion suggestion to add a type name
            } else {
                const VJassToken &typeName = tokens.at(i);

                if (typeName.isValidIdentifier()) {
                    i++;

                    if (i == tokens.size()) {
                        vjassType->addErrorAtEndOf(typeName, "Missing keyword extends for type identifier (only type handle is declared implicitely): " + typeName.getValue());
                        VJassKeyword *extendsKeyword = new VJassKeyword(typeName.getLine(), typeName.getColumn());
                        extendsKeyword->setKeyword(VJassToken::KEYWORD_EXTENDS);
                        ast.addCodeCompletionSuggestion(extendsKeyword);
                    } else {
                        const VJassToken &extendsKeyword = tokens.at(i);

                        if (extendsKeyword.getType() != VJassToken::ExtendsKeyword) {
                            vjassType->addError(extendsKeyword, "Expected extends keyword instead of: " + typeName.getValue());
                            // TODO add code completion suggestion replace extendsToken with extends
                        } else {
                            i++;

                            if (i == tokens.size()) {
                                vjassType->addErrorAtEndOf(extendsKeyword, "Missing parent type for type " + typeName.getValue());
                                // TODO add code completion suggestion to add a type name
                            } else {
                                const VJassToken &parentType = tokens.at(i);

                                if (!parentType.isValidIdentifier()) {
                                    vjassType->addError(parentType, "Invalid parent type identifier " + parentType.getValue());
                                }

                                i++;
                            }
                        }
                    }

                } else {
                    vjassType->addError(typeName, "Invalid type identifier: " + typeName.getValue());
                    // TODO add code completion suggestion replace typeName with valid identifier
                }
            }

            ast.addChild(vjassType);
        // function
        } else if (token.getType() == VJassToken::FunctionKeyword) {
            VJassFunction *vjassFunction = new VJassFunction(token.getLine(), token.getColumn());

            // TODO Depends on where it is done
            if (isInFunction) {
                vjassFunction->addError(token, "Cannot declare function inside of function.");
            }

            isInFunction = true;
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
                                bool expectMoreParameters = true;
                                int parameterCounter = 0;

                                if (i + 1 < tokens.size()) {
                                    for (i = i + 1; i < tokens.size() && !gotError; ) {
                                        const VJassToken &parameterType = tokens.at(i);

                                        if  (parameterType.getType() == VJassToken::NothingKeyword) {
                                            if (parameterCounter > 0) {
                                                vjassFunction->addErrorAtEndOf(parameterType, "Unexpected nothing keyword");
                                            }

                                            break;
                                        } else if (i == tokens.size() - 1) {
                                            vjassFunction->addErrorAtEndOf(parameterType, "Missing parameter name for parameter type " + parameterType.getValue());
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

                                        const int tmpIndex = i + 1;

                                        if (tmpIndex < tokens.size()) {
                                            const VJassToken &separatorToken = tokens.at(tmpIndex);

                                            if (separatorToken.getType() == VJassToken::Separator) {
                                                expectMoreParameters = true;
                                                i = tmpIndex;
                                            } else if (separatorToken.getType() != VJassToken::ReturnsKeyword) {
                                                vjassFunction->addError(separatorToken, "Expected , but got  " + separatorToken.getValue());
                                                gotError = true;
                                            } else {
                                                expectMoreParameters = false;
                                            }
                                        } else {
                                            expectMoreParameters = false;
                                        }
                                    }
                                }

                                if (!gotError) {
                                    if (expectMoreParameters) {
                                        vjassFunction->addErrorAtEndOf(tokens.at(i - 1), "Expected more parameters!");
                                    } else {
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
                isInFunction = false;
            } else {
                ast.addError(token, "Expected after defining a function before using: " + token.getValue());
            }
        } else if (token.getType() == VJassToken::Comment) {
            ast.addComment(token.getValue());
        } else if (token.getType() == VJassToken::LineBreak) {
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
        // all keywords not handled at this point should be invalid here
        } else if (token.isValidKeyword()) {
            ast.addError(token, "Unexpected keyword: " + token.getValue());
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
                 const VJassToken &startingToken = tokens.at(i);

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

        VJassKeyword *nativeKeyword = new VJassKeyword(0, 0);
        nativeKeyword->setKeyword(VJassToken::KEYWORD_NATIVE);
        ast.addCodeCompletionSuggestion(nativeKeyword);

        VJassKeyword *typeKeyword = new VJassKeyword(0, 0);
        typeKeyword->setKeyword(VJassToken::KEYWORD_TYPE);
        ast.addCodeCompletionSuggestion(typeKeyword);
    }

    return ast;
}
