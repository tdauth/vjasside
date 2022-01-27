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
#include "vjassexpression.h"
#include "vjassstatement.h"
#include "vjasslocalstatement.h"
#include "vjasssetstatement.h"
#include "vjassifstatement.h"

VJassParser::VJassParser()
{
}

namespace {

inline bool hasReachedEndOfLine(const QList<VJassToken> &tokens, int i, bool &wasLineBreak) {
    bool result = i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak ||  tokens.at(i).getType() == VJassToken::Comment;

    if (i < tokens.size() && tokens.at(i).getType() == VJassToken::LineBreak) {
        wasLineBreak = true;
    }

    return result;
}

inline bool hasReachedEndOfLine(const QList<VJassToken> &tokens, int i) {
    bool wasLineBreak = false;

    return hasReachedEndOfLine(tokens, i, wasLineBreak);
}

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

                                i++;
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

                                i += 2;
                            }

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

                                if (returnType.getType() != VJassToken::NothingKeyword && !returnType.isValidType()) {
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

/**
 * @brief parseExpression
 *
 * Expressions are something like: ( ( x + y) / myFunction(10) )
 * This function parses expressions from left to right and the nesting recursively.
 * TODO It can check valid operations for literals but not for identifiers since their types are not known at parsing.
 *
 * @param tokens
 * @param token
 * @param ast
 * @param i
 */
inline VJassExpression* parseExpression(const QList<VJassToken> &tokens, const VJassToken &token, VJassAst *ast, int &i) {
    VJassExpression *result = nullptr;
    i++;

    if (i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak) {
        ast->addErrorAtEndOf(token, QObject::tr("Missing expression."));
    } else {
        const VJassToken &nextToken = tokens.at(i);

        // TODO Can be a function call as well. Get all parameters in between?
        if (nextToken.getType() == VJassToken::LeftBracket) {
            int rightBracketIndex = -1;
            qDebug() << "Left bracket at" << i;
            int j = i + 1;

            for ( ; j < tokens.size() && rightBracketIndex == -1; ++j) {
                const VJassToken &rightBracket = tokens.at(j);

                if (rightBracket.getType() == VJassToken::RightBracket) {
                    qDebug() << "Found right bracket at" << j;
                    rightBracketIndex = j;
                } else if (rightBracket.getType() == VJassToken::LineBreak) {
                    qDebug() << "Found line break" << j;
                    ast->addErrorAtEndOf(nextToken, QObject::tr("Missing right bracket for left bracket."));

                    break;
                }
            }

            if (rightBracketIndex == -1) {
                ast->addErrorAtEndOf(nextToken, QObject::tr("Missing right bracket for left bracket."));
            } else {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn()) ;
                result->setType(VJassExpression::Brackets);

                // get all expressions in between the brackets
                if (j > i + 1) {
                    qDebug() << "Parsing expression between brackets" << i;
                    VJassAst *child = parseExpression(tokens, nextToken, result, i);

                    if (child != nullptr) {
                        result->addChild(child);
                    } else {
                        result->addErrorAtEndOf(nextToken, QObject::tr("Empty expression between brackets"));
                    }
                } else {
                    result->addErrorAtEndOf(nextToken, QObject::tr("Empty expression between brackets"));
                }

                // skip the right bracket
                if (rightBracketIndex != -1 && i < rightBracketIndex) {
                    i++;
                }
            }
        // operator
        } else if (nextToken.getType() == VJassToken::Operator) {
            // TODO only allow + and - as infix

        // identifier
        } else if (nextToken.getType() == VJassToken::Text) {
            const int j = i + 1;

            if (!hasReachedEndOfLine(tokens, j)) {
                const VJassToken &bracket = tokens.at(j);

                // function call
                if (bracket.getType() == VJassToken::LeftBracket) {
                    VJassExpression *functionCall = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                    functionCall->setType(VJassExpression::FunctionCall);

                    VJassExpression *parameters = parseExpression(tokens, token, ast, i);

                    if (parameters != nullptr) {
                        functionCall->addChild(parameters);
                    }

                    result = functionCall;
                // array access
                } else if (bracket.getType() == VJassToken::LeftSquareBracket) {
                    VJassExpression *arrayAccess = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                    arrayAccess->setType(VJassExpression::ArrayAccess);

                    i++;
                    VJassExpression *index = parseExpression(tokens, token, ast, i);

                    if (index != nullptr) {
                        arrayAccess->addChild(index);
                    }

                    result = arrayAccess;
                // operation
                } else if (bracket.getType() == VJassToken::Operator) {
                    VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                    if (bracket.getValue() == "+") {
                        operation->setType(VJassExpression::Sum);
                    } else if (bracket.getValue() == "-") {
                        operation->setType(VJassExpression::Substraction);
                    } else if (bracket.getValue() == "*") {
                        operation->setType(VJassExpression::Multiplication);
                    } else if (bracket.getValue() == "/") {
                        operation->setType(VJassExpression::Division);
                    }

                    i++;
                    VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                    if (rightOperation != nullptr) {
                        operation->addChild(rightOperation);
                    }

                    result = operation;
                } else {
                    ast->addError(bracket, QObject::tr("Unexpected token after identifier: %1").arg(bracket.getValue()));
                }
            }
        } else if (nextToken.getType() == VJassToken::IntegerLiteral) {
            result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
            result->setType(VJassExpression::IntegerLiteral);
            result->setValue(nextToken.getValue());

            const int j = i + 1;

            if (!hasReachedEndOfLine(tokens, j)) {
                const VJassToken &bracket = tokens.at(j);

                if (bracket.getType() == VJassToken::Operator) {
                    VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                    if (bracket.getValue() == "+") {
                        operation->setType(VJassExpression::Sum);
                    } else if (bracket.getValue() == "-") {
                        operation->setType(VJassExpression::Substraction);
                    } else if (bracket.getValue() == "*") {
                        operation->setType(VJassExpression::Multiplication);
                    } else if (bracket.getValue() == "/") {
                        operation->setType(VJassExpression::Division);
                    }

                    i++;
                    VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                    if (rightOperation != nullptr) {
                        operation->addChild(rightOperation);
                    }

                    result = operation;
                } else {
                    ast->addError(bracket, QObject::tr("Unexpected token after integer literal: %1").arg(bracket.getValue()));
                }
            }
        } else if (nextToken.getType() == VJassToken::RealLiteral) {
            result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
            result->setType(VJassExpression::RealLiteral);
            result->setValue(nextToken.getValue());

            const int j = i + 1;

            if (!hasReachedEndOfLine(tokens, j)) {
                const VJassToken &bracket = tokens.at(j);

                if (bracket.getType() == VJassToken::Operator) {
                    VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                    if (bracket.getValue() == "+") {
                        operation->setType(VJassExpression::Sum);
                    } else if (bracket.getValue() == "-") {
                        operation->setType(VJassExpression::Substraction);
                    } else if (bracket.getValue() == "*") {
                        operation->setType(VJassExpression::Multiplication);
                    } else if (bracket.getValue() == "/") {
                        operation->setType(VJassExpression::Division);
                    }

                    i++;
                    VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                    if (rightOperation != nullptr) {
                        operation->addChild(rightOperation);
                    }

                    result = operation;
                } else {
                    ast->addError(bracket, QObject::tr("Unexpected token after integer literal: %1").arg(bracket.getValue()));
                }
            }
        } else if (nextToken.getType() == VJassToken::StringLiteral) {
            result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
            result->setType(VJassExpression::StringLiteral);
            result->setValue(nextToken.getValue());

            const int j = i + 1;

            if (!hasReachedEndOfLine(tokens, j)) {
                const VJassToken &bracket = tokens.at(j);

                if (bracket.getType() == VJassToken::Operator) {
                    if (bracket.getValue() == "+") {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        operation->setType(VJassExpression::Sum);

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    } else {
                        ast->addError(bracket, QObject::tr("Unexpected operation after string literal: %1").arg(bracket.getValue()));
                    }
                } else {
                    ast->addError(bracket, QObject::tr("Unexpected token after integer literal: %1").arg(bracket.getValue()));
                }
            }
        } else if (nextToken.getType() == VJassToken::TrueKeyword) {
            result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
            result->setType(VJassExpression::True);
            result->setValue(nextToken.getValue());

            // TODO Look for operators
        } else if (nextToken.getType() == VJassToken::FalseKeyword) {
            result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
            result->setType(VJassExpression::False);
            result->setValue(nextToken.getValue());

            // TODO Look for operators
        // TODO Check keywords like not and boolean operators
        } else {
            ast->addError(nextToken, "Invalid expression: " + nextToken.getValue());
        }
    }

    return result;
}

}

VJassAst* VJassParser::parse(const QList<VJassToken> &tokens) {
    VJassAst *ast = new VJassAst(0, 0);
    bool isInFunction = false;
    bool afterLocalsInFunction = false;
    QStack<VJassIfStatement*> ifStatements;
    VJassFunction *currentFunction = nullptr;
    bool isInGlobals = false;
    VJassGlobals *currentGlobals = nullptr;

    for (int i = 0; i < tokens.size(); i++) {
        const VJassToken &token = tokens.at(i);
        bool wasLineBreak = false;

        if (token.getType() == VJassToken::LineBreak) {
            // do nothing, just consume them
            wasLineBreak = true;
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
                                    parseExpression(tokens, assignmentOperatorToken, global, i);
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
                afterLocalsInFunction = false;

                if (!ifStatements.isEmpty()) {
                    ast->addError(token, QObject::tr("%1 unclosed if statements").arg(ifStatements.size()));
                }

                ifStatements.clear(); // remove unclosed if statements
            } else {
                ast->addError(token, "Expected after defining a function before using: " + token.getValue());
            }
        } else if (token.getType() == VJassToken::LocalKeyword) {
            if (!isInFunction) {
                ast->addError(token, QObject::tr("Keyword local is only allowed inside of a function"));
            } else if (afterLocalsInFunction) {
                ast->addError(token, QObject::tr("Keyword local is only allowed at the beginning of the function"));
            } else {
                VJassLocalStatement *localStatement = new VJassLocalStatement(token.getLine(), token.getColumn());

                i++;

                if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                    ast->addErrorAtEndOf(token, QObject::tr("Expected type name."));
                } else {
                    const VJassToken &typeName = tokens.at(i);

                    if (!typeName.isValidType()) {
                        ast->addError(typeName, QObject::tr("Invalid type name %1").arg(typeName.getValue()));
                    } else {
                        localStatement->setType(typeName.getValue());

                        i++;

                        if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                            ast->addErrorAtEndOf(typeName, QObject::tr("Expected local variable name."));
                        } else {
                            const VJassToken &variableIdentifier = tokens.at(i);

                            if (!variableIdentifier.isValidIdentifier()) {
                                ast->addError(variableIdentifier, QObject::tr("Invalid variable name %1").arg(variableIdentifier.getValue()));
                            } else {
                                localStatement->setVariableName(variableIdentifier.getValue());

                                i++;

                                // assignment is optional
                                if (!hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                                    const VJassToken &assignmentOperator = tokens.at(i);

                                    if (assignmentOperator.getType() != VJassToken::AssignmentOperator) {
                                        ast->addError(assignmentOperator, QObject::tr("Invalid assignment operator %1").arg(variableIdentifier.getValue()));
                                    } else {
                                        VJassExpression *expression = parseExpression(tokens, assignmentOperator, ast, i);

                                        if (expression != nullptr) {
                                            localStatement->addChild(expression);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                currentFunction->addChild(localStatement);
            }
        } else if (token.getType() == VJassToken::SetKeyword) {
            if (!isInFunction) {
                ast->addError(token, QObject::tr("Keyword set is only allowed inside of a function"));
            } else {
                afterLocalsInFunction = true;
                VJassSetStatement *setStatement = new VJassSetStatement(token.getLine(), token.getColumn());

                i++;

                if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                    ast->addErrorAtEndOf(token, QObject::tr("Expected variable name."));
                } else {
                    const VJassToken &variableName = tokens.at(i);

                    if (variableName.isValidIdentifier()) {
                        ast->addError(variableName, QObject::tr("Invalid variable name %1").arg(variableName.getValue()));
                    } else {
                        i++;

                        if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                            ast->addErrorAtEndOf(variableName, QObject::tr("Expected assignment operator."));
                        } else {
                            const VJassToken &assignmentOperator = tokens.at(i);

                            if (assignmentOperator.getType() != VJassToken::AssignmentOperator) {
                                ast->addError(assignmentOperator, QObject::tr("Invalid assignment operator of %1").arg(assignmentOperator.getValue()));
                            } else {
                                i++;

                                if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                                    ast->addErrorAtEndOf(assignmentOperator, QObject::tr("Expected assignment expression."));
                                } else {
                                    VJassExpression *expression = parseExpression(tokens, token, ast, i);

                                    if (expression == nullptr) {
                                        ast->addErrorAtEndOf(assignmentOperator, QObject::tr("Expected assignment expression."));
                                    } else {
                                        setStatement->addChild(expression);
                                    }
                                }
                            }
                        }
                    }
                }

                currentFunction->addChild(setStatement);
            }
        } else if (token.getType() == VJassToken::IfKeyword) {
            if (!isInFunction) {
                ast->addError(token, QObject::tr("Keyword if is only allowed inside of a function"));
            } else {
                afterLocalsInFunction = true;
                VJassIfStatement *ifStatement = new VJassIfStatement(token.getLine(), token.getColumn());

                VJassExpression *expression = parseExpression(tokens, token, ast, i);

                if (expression != nullptr) {
                    ifStatement->addChild(expression);
                }

                i++;

                if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                    ast->addErrorAtEndOf(token, QObject::tr("Expected then keyword."));
                } else {
                    const VJassToken &thenToken = tokens.at(i);

                    if (thenToken.getType() != VJassToken::ThenKeyword) {
                        ast->addErrorAtEndOf(thenToken, QObject::tr("Expected then keyword instead of %1").arg(thenToken.getValue()));
                    }
                }

                currentFunction->addChild(ifStatement);
                ifStatements.push_back(ifStatement);
            }
        } else if (token.getType() == VJassToken::EndifKeyword) {
            if (!isInFunction) {
                ast->addError(token, QObject::tr("Unexpected endif outside of function body"));
            }

            if (ifStatements.isEmpty()) {
                ast->addError(token, QObject::tr("Unexpected endif keyword"));
            } else {
                ifStatements.pop_back();
            }
        } else if (token.getType() == VJassToken::Comment) {
            ast->addComment(token.getValue());
        } else if (token.getType() == VJassToken::Unknown) {
            ast->addError(token, QObject::tr("Unknown text %1").arg(token.getValue()));
        } else if (token.getType() == VJassToken::Text) {
            suggestLineStartKeywords(isInFunction, isInGlobals, *ast, &token);

            ast->addError(token, QObject::tr("Unknown text %1").arg(token.getValue()));
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
            if (ifStatements.size() > 0) {
                ast->addErrorAtEndOf(tokens.last(), QObject::tr("%1 unclosed if statements").arg(ifStatements.size()));
            }

            if (isInGlobals) {
                ast->addErrorAtEndOf(tokens.last(), QObject::tr("Missing endglobals"));
            } else if (isInFunction) {
                ast->addErrorAtEndOf(tokens.last(), QObject::tr("Missing endfunction"));
            }
        }
    }

    // suggest auto completions in a new empty document
    if (tokens.isEmpty()) {
        suggestLineStartKeywords(isInFunction, isInGlobals, *ast, nullptr);
    }

    return ast;
}
