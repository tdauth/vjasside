#include <QtCore>

#include "vjassparser.h"
#include "vjassscanner.h"
#include "vjassast.h"
#include "vjassfunction.h"
#include "vjassnative.h"
#include "vjasskeyword.h"
#include "vjasstype.h"
#include "vjassglobals.h"

VJassParser::VJassParser()
{
}

namespace {

inline void suggestLineStartKeywords(bool isInFunction, const VJassToken &token, VJassAst &ast) {
    QList<QString> keywords;

    if (!isInFunction) {
        keywords.push_back(VJassToken::KEYWORD_FUNCTION);
        keywords.push_back(VJassToken::KEYWORD_GLOBALS);
        keywords.push_back(VJassToken::KEYWORD_CONSTANT);
        keywords.push_back("//");
        keywords.push_back("/*");
        keywords.push_back(VJassToken::KEYWORD_NATIVE);
        keywords.push_back(VJassToken::KEYWORD_TYPE);
    } else {
        keywords.push_back(VJassToken::KEYWORD_ENDFUNCTION);
    }

    for (const QString &keyword : keywords) {
        if (keyword.startsWith(token.getValue())) {
            VJassKeyword *functionKeyword = new VJassKeyword(token.getLine(), token.getColumn() + token.getValue().size());
            functionKeyword->setKeyword(keyword);
            ast.addCodeCompletionSuggestion(functionKeyword);
        }
    }
}

inline void parseFunctionDeclaration(const QList<VJassToken> &tokens, const VJassToken &token, VJassNative *vjassFunction, VJassAst *ast, int &i) {
    i++;

    if (i == tokens.size()) {
        vjassFunction->addErrorAtEndOf(token, "Missing function declaration identifier.");
    } else {
        const VJassToken &identifier = tokens.at(i);

        if (identifier.isValidIdentifier()) {
            vjassFunction->setIdentifier(identifier.getValue());
            i++;

            if (i == tokens.size()) {
                vjassFunction->addError(identifier, "Missing takes keyword.");
                VJassKeyword *takesKeyword = new VJassKeyword(identifier.getLine(), identifier.getColumn());
                takesKeyword->setKeyword(VJassToken::KEYWORD_TAKES);
                ast->addCodeCompletionSuggestion(takesKeyword);
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

                        while (i < tokens.size() && !gotError && expectMoreParameters) {
                            const VJassToken &parameterType = tokens.at(i);

                            if  (parameterType.getType() == VJassToken::NothingKeyword) {
                                if (parameterCounter > 0) {
                                    vjassFunction->addErrorAtEndOf(parameterType, "Unexpected nothing keyword");
                                    gotError = true;
                                } else {
                                    expectMoreParameters = false;
                                }
                            } else {
                                if (!parameterType.isValidType()) {
                                    vjassFunction->addErrorAtEndOf(parameterType, "Invalid parameter type: " + parameterType.getValue());
                                }

                                // no parameter name
                                if (i == tokens.size() - 1) {
                                    vjassFunction->addErrorAtEndOf(parameterType, "Missing parameter name for parameter type " + parameterType.getValue());
                                    gotError = true;
                                } else {
                                    const VJassToken &parameterName = tokens.at(i + 1);

                                    if (parameterName.isValidIdentifier()) {
                                        vjassFunction->addParameter(parameterType.getValue(), parameterName.getValue());
                                    } else {
                                        vjassFunction->addErrorAtEndOf(parameterName, "Invalid parameter name: " + parameterName.getValue());
                                        gotError = true;
                                    }
                                }
                            }

                            i += 2;
                            parameterCounter++;

                            if (i < tokens.size()) {
                                const VJassToken &separatorToken = tokens.at(i);

                                // , for the next parameter
                                if (separatorToken.getType() == VJassToken::Separator) {
                                    expectMoreParameters = true;
                                    i++;
                                // something unexpected instead of , or returns
                                } else if (separatorToken.getType() != VJassToken::ReturnsKeyword) {
                                    vjassFunction->addError(separatorToken, "Expected , but got  " + separatorToken.getValue());
                                    gotError = true;
                                // returns
                                } else {
                                    expectMoreParameters = false;
                                    i++;
                                }
                            // nothing left -> missing return
                            } else if (!gotError) {
                                expectMoreParameters = false;
                                gotError = true;
                                vjassFunction->addError(tokens.at(i - 1), QObject::tr("Expected returns or , character for more parameters"));
                            }
                        }

                        // return type
                        if (!gotError) {
                            if (expectMoreParameters) {
                                vjassFunction->addErrorAtEndOf(tokens.at(i - 1), QObject::tr("Expecting more parameters"));
                            } else if (i >= tokens.size()) {
                                vjassFunction->addErrorAtEndOf(tokens.at(i - 1), QObject::tr("Missing return type"));
                            } else {
                                const VJassToken &returnType = tokens.at(i);
                                vjassFunction->setReturnType(returnType.getValue());

                                if (!returnType.isValidType()) {
                                    vjassFunction->addErrorAtEndOf(returnType, QObject::tr("Invalid return type: %1").arg(returnType.getValue()));
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
}

}

VJassAst* VJassParser::parse(const QList<VJassToken> &tokens) {
    VJassAst *ast = new VJassAst(0, 0);
    bool isInFunction = false;
    VJassFunction *currentFunction = nullptr;
    bool isInGlobals = false;
    VJassGlobals *currentGlobals = nullptr;

    for (int i = 0; i < tokens.size(); i++) {
        const VJassToken &token = tokens.at(i);
        bool wasLineBreak = false;

        // new line
        if (token.getType() == VJassToken::LineBreak) {
            // do nothing, just consume them
            wasLineBreak = true;
        // type
        } else if (token.getType() == VJassToken::TypeKeyword) {
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
                        ast->addCodeCompletionSuggestion(extendsKeyword);
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

            ast->addChild(vjassType);
        // constant
        } else if (token.getType() == VJassToken::ConstantKeyword) {
            if (isInGlobals) {
                // TODO parse constant
                ast->addErrorAtEndOf(token, QObject::tr("Constants are not supported yet."));
            } else {
                i++;

                if (i >= tokens.size()) {
                    ast->addErrorAtEndOf(token, QObject::tr("Expected either native or function after %1").arg(token.getValue()));
                } else {
                    const VJassToken &functionKeyword = tokens.at(i);

                    if (functionKeyword.getType() == VJassToken::FunctionKeyword) {
                        VJassFunction *vjassFunction = new VJassFunction(token.getLine(), token.getColumn());

                        // TODO Depends on where it is done
                        if (isInFunction) {
                            vjassFunction->addError(token, "Cannot declare function inside of function.");
                        }

                        isInFunction = true;
                        currentFunction = vjassFunction;
                        parseFunctionDeclaration(tokens, token, vjassFunction, ast, i);

                        ast->addChild(vjassFunction);
                    } else if (functionKeyword.getType() == VJassToken::NativeKeyword) {
                        VJassNative *vjassNative = new VJassNative(token.getLine(), token.getColumn());

                        if (isInFunction) {
                            vjassNative->addError(token, QObject::tr("Cannot declare native inside of function."));
                        }

                        parseFunctionDeclaration(tokens, token, vjassNative, ast, i);

                        ast->addChild(vjassNative);
                    } else {
                        ast->addError(functionKeyword, QObject::tr("Expected either native or function instead of %1.").arg(functionKeyword.getValue()));
                    }
                }
            }
        // native
        } else if (token.getType() == VJassToken::NativeKeyword) {
            VJassNative *vjassNative = new VJassNative(token.getLine(), token.getColumn());

            if (isInFunction) {
                vjassNative->addError(token, QObject::tr("Cannot declare native inside of function."));
            } else if (isInGlobals) {
                vjassNative->addError(token, QObject::tr("Cannot declare native inside of globals."));
            }

            parseFunctionDeclaration(tokens, token, vjassNative, ast, i);

            ast->addChild(vjassNative);
        // globals
        } else if (token.getType() == VJassToken::GlobalsKeyword) {
            VJassGlobals *vjassGlobals = new VJassGlobals(token.getLine(), token.getColumn());

            if (isInFunction) {
                vjassGlobals->addError(token, QObject::tr("Cannot declare globals inside of function."));
            } else if (isInGlobals) {
                vjassGlobals->addError(token, QObject::tr("Cannot declare globals inside of globals."));
            } else {
                isInGlobals = true;
                currentGlobals = vjassGlobals;
            }

            ast->addChild(vjassGlobals);
        // endglobals
        } else if (token.getType() == VJassToken::EndglobalsKeyword) {
            if (!isInGlobals) {
                ast->addError(token, QObject::tr("Unable to close globals when no globals were declared."));
            }

            isInGlobals = false;
            currentGlobals = nullptr;
        // function
        } else if (token.getType() == VJassToken::FunctionKeyword) {
            VJassFunction *vjassFunction = new VJassFunction(token.getLine(), token.getColumn());

            // TODO Depends on where it is done. It can be used in expressions
            if (isInFunction) {
                vjassFunction->addError(token,  QObject::tr("Cannot declare function inside of function."));
            } else if (isInGlobals) {
                vjassFunction->addError(token, QObject::tr("Cannot declare function inside of globals."));
            }

            isInFunction = true;
            currentFunction = vjassFunction;
            parseFunctionDeclaration(tokens, token, vjassFunction, ast, i);

            ast->addChild(vjassFunction);
        } else if (token.getType() == VJassToken::EndfunctionKeyword) {
            if (isInFunction) {
                isInFunction = false;
                currentFunction = nullptr;
            } else {
                ast->addError(token, "Expected after defining a function before using: " + token.getValue());
            }
        } else if (token.getType() == VJassToken::Comment) {
            ast->addComment(token.getValue());
        } else if (token.getType() == VJassToken::LineBreak) {
        } else if (token.getType() == VJassToken::Unknown) {
            ast->addError(token, "Unknown symbol: " + token.getValue());
        } else if (token.getType() == VJassToken::Text) {
            suggestLineStartKeywords(isInFunction, token, *ast);
        // all keywords not handled at this point should be invalid here
        } else if (token.isValidKeyword()) {
            ast->addError(token, "Unexpected keyword: " + token.getValue());
        }

        // add comments to the last AST child
        // comments might be useful attached to their declarations to give some information
        if (!wasLineBreak) {
            const int commentsIndex = i + 1;

            if (tokens.size() > commentsIndex) {
                const VJassToken &commentsToken = tokens.at(commentsIndex);
                VJassAst *child = ast->getChildren().isEmpty() ? ast : ast->getChildren().last();

                if (commentsToken.getType() != VJassToken::Comment && commentsToken.getType() != VJassToken::LineBreak) {
                    child->addError(commentsToken, QObject::tr("Expected comment or line break instead of %1").arg(commentsToken.getValue()));
                } else if (commentsToken.getType() == VJassToken::Comment) {
                    child->addComment(commentsToken.getValue());
                // line break
                } else {
                }
            }
        }
    }

    // suggest auto completions in a new empty document
    if (tokens.isEmpty()) {
        if (!isInFunction) {
            VJassKeyword *functionKeyword = new VJassKeyword(0, 0);
            functionKeyword->setKeyword(VJassToken::KEYWORD_FUNCTION);
            ast->addCodeCompletionSuggestion(functionKeyword);

            VJassKeyword *globalsKeyword = new VJassKeyword(0, 0);
            globalsKeyword->setKeyword(VJassToken::KEYWORD_GLOBALS);
            ast->addCodeCompletionSuggestion(globalsKeyword);

            VJassKeyword *constantKeyword = new VJassKeyword(0, 0);
            constantKeyword->setKeyword(VJassToken::KEYWORD_CONSTANT);
            ast->addCodeCompletionSuggestion(constantKeyword);
        }

        VJassKeyword *commentLineKeyword = new VJassKeyword(0, 0);
        commentLineKeyword->setKeyword("//");
        ast->addCodeCompletionSuggestion(commentLineKeyword);

        VJassKeyword *commentBlockKeyword = new VJassKeyword(0, 0);
        commentBlockKeyword->setKeyword("/*");
        ast->addCodeCompletionSuggestion(commentBlockKeyword);

        if (!isInFunction) {
            VJassKeyword *nativeKeyword = new VJassKeyword(0, 0);
            nativeKeyword->setKeyword(VJassToken::KEYWORD_NATIVE);
            ast->addCodeCompletionSuggestion(nativeKeyword);

            VJassKeyword *typeKeyword = new VJassKeyword(0, 0);
            typeKeyword->setKeyword(VJassToken::KEYWORD_TYPE);
            ast->addCodeCompletionSuggestion(typeKeyword);
        }
    }

    return ast;
}
