#include <QtCore>

#include "vjassparser.h"
#include "vjassscanner.h"
#include "vjassast.h"
#include "vjassfunction.h"
#include "vjassnative.h"
#include "vjasskeyword.h"
#include "vjasstype.h"
#include "vjassglobals.h"
#include "vjassglobal.h"

VJassParser::VJassParser()
{
}

namespace {

inline void suggestLineStartKeywords(bool isInFunction, bool isInGlobals, VJassAst &ast, const VJassToken *token) {
    QList<QString> keywords;

    if (!isInFunction && !isInGlobals) {
        keywords.push_back(VJassToken::KEYWORD_FUNCTION);
        keywords.push_back(VJassToken::KEYWORD_GLOBALS);
        keywords.push_back(VJassToken::KEYWORD_CONSTANT);
        keywords.push_back("//");
        keywords.push_back("/*");
        keywords.push_back(VJassToken::KEYWORD_NATIVE);
        keywords.push_back(VJassToken::KEYWORD_TYPE);
    } else if (isInFunction) {
        keywords.push_back(VJassToken::KEYWORD_ENDFUNCTION);
    } else if (isInGlobals) {
        keywords.push_back(VJassToken::KEYWORD_ENDGLOBALS);
    }

    const int line = token == nullptr ? 0 : token->getLine();
    const int column = token == nullptr ? 0 : token->getColumn() + token->getValue().size();

    for (const QString &keyword : keywords) {
        if (token == nullptr || keyword.startsWith(token->getValue())) {
            VJassKeyword *functionKeyword = new VJassKeyword(line, column);
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
                //ast->addErrorAtEndOf(token, QObject::tr("Constants are not supported yet."));

                // TODO Extract into function which can be used for constants and global variables
                i++;

                if (i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak) {
                    ast->addErrorAtEndOf(token, QObject::tr("Missing type after constant keyword."));
                } else {
                    VJassGlobal *global = new VJassGlobal(token.getLine(), token.getColumn());
                    global->setIsConstant(true);
                    const VJassToken &typeToken = tokens.at(i);

                    if (!typeToken.isValidType()) {
                        ast->addError(typeToken, QObject::tr("Invalid type %1").arg(typeToken.getValue()));
                    } else {
                        global->setType(typeToken.getValue());
                    }

                    i++;

                    if (i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak) {
                        if (global->getIsConstant()) {
                            ast->addErrorAtEndOf(typeToken, QObject::tr("Missing name of global variable."));
                        } else {
                            ast->addErrorAtEndOf(typeToken, QObject::tr("Missing array keyword or name of global variable."));
                        }
                    } else {
                        const VJassToken &arrayToken = tokens.at(i);

                        if (arrayToken.getType() == VJassToken::ArrayKeyword) {
                            global->setIsArray(true);
                            i++;
                        }

                        if (i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak) {
                            ast->addErrorAtEndOf(arrayToken, QObject::tr("Missing global variable name."));
                        } else {
                            const VJassToken &nameToken = tokens.at(i);

                            if (!nameToken.isValidIdentifier()) {
                                ast->addError(nameToken, QObject::tr("Invalid identifier for global %1").arg(nameToken.getValue()));
                            } else {
                                global->setName(nameToken.getValue());
                            }

                            i++;

                            // has assignment
                            if (i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak) {
                                if (global->getIsConstant()) {
                                    global->addErrorAtEndOf(nameToken, QObject::tr("Constants require assignment."));
                                }
                            } else {
                                const VJassToken &assignmentOperatorToken = tokens.at(i);

                                if (assignmentOperatorToken.getType() != VJassToken::AssignmentOperator) {
                                    global->addErrorAtEndOf(assignmentOperatorToken, QObject::tr("Expected assignment operator instead of %1").arg(assignmentOperatorToken.getValue()));
                                }

                                if (global->getIsArray()) {
                                    global->addErrorAtEndOf(assignmentOperatorToken, QObject::tr("Assignments of global array variables are not allowed."));
                                } else {
                                    i++;

                                    if (i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak) {
                                        ast->addErrorAtEndOf(arrayToken, QObject::tr("Missing assignment expression."));
                                    } else {
                                        // TODO Parse expression of assignment
                                    }
                                }
                            }
                        }
                    }

                    currentGlobals->addChild(global);
                }
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
            suggestLineStartKeywords(isInFunction, isInGlobals, *ast, &token);
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

        if (i == tokens.size() - 1) {
            if (isInGlobals) {
                ast->addErrorAtEndOf(token, QObject::tr("Missing endglobals"));
            } else if (isInFunction) {
                ast->addErrorAtEndOf(token, QObject::tr("Missing endfunction"));
            }
        }
    }

    // suggest auto completions in a new empty document
    if (tokens.isEmpty()) {
        suggestLineStartKeywords(isInFunction, isInGlobals, *ast, nullptr);
    }

    return ast;
}
