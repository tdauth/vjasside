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

VJassParser::VJassParser()
{
}

namespace {

inline bool hasReachedEndOfLine(const QList<VJassToken> &tokens, int i, bool &wasLineBreak) {
    const bool result = i >= tokens.size() || tokens.at(i).getType() == VJassToken::LineBreak ||  tokens.at(i).getType() == VJassToken::Comment;

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
    QVector<QString> keywords;

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
                                        vjassFunction->addParameter(parameterType.getLine(), parameterType.getColumn(), parameterType.getValue(), parameterName.getValue());
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
 * @param tokens All tokens handled by the parser.
 * @param token The previously handled token from the parser.
 * @param ast The current AST element.
 * @param i The index of the current token handled by the parser.
 * @param bracketsNestingLevel The nesting level starting with 0 of expressions inside expressions with brackets.
 */
inline VJassExpression* parseExpression(const QList<VJassToken> &tokens, const VJassToken &token, VJassAst *ast, int &i, int bracketsNestingLevel = 0, bool required = true) {
    VJassExpression *result = nullptr;
    i++;

    if (hasReachedEndOfLine(tokens, i)) {
        if (required) {
            ast->addErrorAtEndOf(token, QObject::tr("Missing expression after %1").arg(token.getValue()));
        }
    } else {
        const VJassToken &nextToken = tokens.at(i);

        switch (nextToken.getType()) {
            case VJassToken::LeftBracket: {
                // get all right brackets if it is nested to find our nested bracket
                QVector<int> rightBracketIndices;
                bool foundEnd = false;
                qDebug() << "Left bracket at" << i;
                int j = i + 1;

                for ( ; j < tokens.size() && !foundEnd; ++j) {
                    const VJassToken &rightBracket = tokens.at(j);

                    if (rightBracket.getType() == VJassToken::RightBracket) {
                        qDebug() << "Found right bracket at" << j;
                        rightBracketIndices.push_back(j);
                    } else if (rightBracket.getType() == VJassToken::LineBreak || rightBracket.getType() == VJassToken::Comment) {
                        qDebug() << "Found line break" << j;
                        foundEnd = true;
                    }

                    // only parse until the matching right bracket has been found
                    if (!foundEnd) {
                        foundEnd = rightBracketIndices.size() > bracketsNestingLevel;
                    }
                }

                if (rightBracketIndices.size() <= bracketsNestingLevel) {
                    ast->addErrorAtEndOf(nextToken, QObject::tr("Missing right bracket for left bracket."));
                } else {
                   const int rightBracketIndex = rightBracketIndices[rightBracketIndices.size() - bracketsNestingLevel - 1];

                    result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                    result->setType(VJassExpression::Brackets);

                    // get all expressions in between the brackets
                    while (i < rightBracketIndex && !hasReachedEndOfLine(tokens, i)) {
                        qDebug() << "Parsing expression between brackets" << i;

                        VJassAst *child = parseExpression(tokens, nextToken, result, i, bracketsNestingLevel + 1);

                        if (child != nullptr) {
                            result->addChild(child);
                        }
                    }

                    // end after the right bracket token
                    i = rightBracketIndex + 1;
                }

                break;
            }
            // closing brackets should be consumed by the left bracket symbol
            case VJassToken::RightBracket: {
                ast->addErrorAtEndOf(nextToken, QObject::tr("Unexpected right bracket."));

                break;
            }
            // array access
            case VJassToken::LeftSquareBracket: {
                bool foundEnd = false;
                int rightSquareBracketIndex = -1;

                for (int j = i + 1; j < tokens.size() && !foundEnd && rightSquareBracketIndex == -1; ++j) {
                    const VJassToken &rightBracket = tokens.at(j);

                    if (rightBracket.getType() == VJassToken::RightSquareBracket) {
                        qDebug() << "Found right square bracket at" << j;
                        rightSquareBracketIndex = j;
                    } else if (rightBracket.getType() == VJassToken::LineBreak || rightBracket.getType() == VJassToken::Comment) {
                        qDebug() << "Found line break" << j;
                        foundEnd = true;
                    }
                }

                if (rightSquareBracketIndex == -1) {
                    ast->addErrorAtEndOf(nextToken, QObject::tr("Missing closing right square bracket."));
                } else {
                    result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                    result->setType(VJassExpression::ArrayAccess);

                    VJassAst *child = parseExpression(tokens, nextToken, result, i);

                    if (child != nullptr) {
                        result->addChild(child);
                    } else {
                        result->addErrorAtEndOf(nextToken, QObject::tr("Empty or invalid expression between square brackets"));
                    }

                    // end after the right square bracket token
                    i = rightSquareBracketIndex + 1;
                }

                break;
            }
            // closing square brackets should be consumed by the left square bracket symbol
            case VJassToken::RightSquareBracket: {
                ast->addErrorAtEndOf(nextToken, QObject::tr("Unexpected right bracket."));

                break;
            }
            case VJassToken::Operator: {
                if (nextToken.getValue() == "-") {
                    VJassExpression *infixExpression = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                    infixExpression->setType(VJassExpression::Negative);

                    VJassExpression *rightExpression = parseExpression(tokens, nextToken, ast, i);

                    if (rightExpression != nullptr) {
                        infixExpression->addChild(rightExpression);
                    }

                    result = infixExpression;
                } else {
                    ast->addErrorAtEndOf(nextToken, QObject::tr("Unexpected operator: %1").arg(token.getValue()));
                }

                break;
            }
            case VJassToken::Separator: {
                if (hasReachedEndOfLine(tokens, i + 1)) {
                    ast->addErrorAtEndOf(token, QObject::tr("Expected more expressions after separator."));
                } else {
                    result = parseExpression(tokens, nextToken, ast, i);
                }

                break;
            }
            case VJassToken::Text: {
                const int j = i + 1;

                if (!hasReachedEndOfLine(tokens, j)) {
                    const VJassToken &bracket = tokens.at(j);

                    // function call
                    if (bracket.getType() == VJassToken::LeftBracket) {
                        VJassExpression *functionCall = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                        functionCall->setType(VJassExpression::FunctionCall);
                        functionCall->setValue(nextToken.getValue());

                        VJassExpression *parameters = parseExpression(tokens, token, ast, i);

                        if (parameters != nullptr) {
                            functionCall->addChild(parameters);
                        }

                        result = functionCall;
                    // parameters
                    } else if (bracket.getType() == VJassToken::Separator) {
                        i++;
                        result = parseExpression(tokens, token, ast, i);
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
                    // comparison
                    } else if (bracket.getType() == VJassToken::ComparisonOperator) {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        if (bracket.getValue() == "==") {
                            operation->setType(VJassExpression::Equals);
                        } else if (bracket.getValue() == "!=") {
                            operation->setType(VJassExpression::NotEquals);
                        } else if (bracket.getValue() == ">") {
                            operation->setType(VJassExpression::GreaterThan);
                        } else if (bracket.getValue() == "<") {
                            operation->setType(VJassExpression::LessThan);
                        } else if (bracket.getValue() == "<=") {
                            operation->setType(VJassExpression::LessThanOrEquals);
                        } else if (bracket.getValue() == ">=") {
                            operation->setType(VJassExpression::GreaterThanOrEquals);
                        }

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    // concatenation
                    } else if (bracket.getType() == VJassToken::AndKeyword || bracket.getType() == VJassToken::OrKeyword) {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        if (bracket.getType() == VJassToken::AndKeyword) {
                            operation->setType(VJassExpression::And);
                        } else {
                            operation->setType(VJassExpression::Or);
                        }

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    } else if (bracket.getType() != VJassToken::ThenKeyword) {
                        ast->addError(bracket, QObject::tr("Unexpected token after identifier: %1").arg(bracket.getValue()));
                    } else {
                        // consume the identifier
                        i++;
                    }
                // identifier only (for example on return or an if statement with only a boolean variable)
                } else {
                    result = new VJassExpression(token.getLine(), token.getColumn());
                    result->setType(VJassExpression::Identifier);
                    result->setValue(token.getValue());
                }

                break;
            }
            case VJassToken::IntegerLiteral: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::IntegerLiteral);
                result->setValue(nextToken.getValue());

                i++;

                if (!hasReachedEndOfLine(tokens, i)) {
                    const VJassToken &bracket = tokens.at(i);

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

                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    // closing right bracket
                    } else if (bracket.getType() == VJassToken::RightBracket || bracket.getType() == VJassToken::RightSquareBracket || bracket.getType() == VJassToken::ThenKeyword || bracket.getType() == VJassToken::Separator) {
                    } else {
                        ast->addError(bracket, QObject::tr("Unexpected token after integer literal: %1").arg(bracket.getValue()));
                    }
                }

                break;
            }
            case VJassToken::RealLiteral: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::RealLiteral);
                result->setValue(nextToken.getValue());

                i++;

                if (!hasReachedEndOfLine(tokens, i)) {
                    const VJassToken &bracket = tokens.at(i);

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

                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    // closing right bracket
                    } else if (bracket.getType() == VJassToken::RightBracket || bracket.getType() == VJassToken::ThenKeyword || bracket.getType() == VJassToken::Separator) {
                    } else {
                        ast->addError(bracket, QObject::tr("Unexpected token after real literal: %1").arg(bracket.getValue()));
                    }
                }

                break;
            }
            case VJassToken::RawCodeLiteral: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::RawCodeLiteral);
                result->setValue(nextToken.getValue());

                i++;

                break;
            }
            case VJassToken::StringLiteral: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::StringLiteral);
                result->setValue(nextToken.getValue());

                i++;

                if (!hasReachedEndOfLine(tokens, i)) {
                    const VJassToken &bracket = tokens.at(i);

                    if (bracket.getType() == VJassToken::Operator) {
                        if (bracket.getValue() == "+") {
                            VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                            operation->setType(VJassExpression::Sum);

                            VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                            if (rightOperation != nullptr) {
                                operation->addChild(rightOperation);
                            }

                            result = operation;
                        } else {
                            ast->addError(bracket, QObject::tr("Unexpected operation after string literal: %1").arg(bracket.getValue()));
                        }
                    } else {
                        ast->addError(bracket, QObject::tr("Unexpected token after string literal: %1").arg(bracket.getValue()));
                    }
                }

                break;
            }
            case VJassToken::TrueKeyword: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::True);
                result->setValue(nextToken.getValue());

                const int j = i + 1;

                if (!hasReachedEndOfLine(tokens, j)) {
                    const VJassToken &bracket = tokens.at(j);

                    // comparison
                    if (bracket.getType() == VJassToken::ComparisonOperator && (bracket.getValue() == "==" || bracket.getValue() == "!=")) {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        if (bracket.getValue() == "==") {
                            operation->setType(VJassExpression::Equals);
                        } else if (bracket.getValue() == "!=") {
                            operation->setType(VJassExpression::NotEquals);
                        }

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    // concatenation
                    } else if (bracket.getType() == VJassToken::AndKeyword || bracket.getType() == VJassToken::OrKeyword) {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        if (bracket.getType() == VJassToken::AndKeyword) {
                            operation->setType(VJassExpression::And);
                        } else {
                            operation->setType(VJassExpression::Or);
                        }

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    }
                }

                break;
            }
            case VJassToken::FalseKeyword: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::False);
                result->setValue(nextToken.getValue());

                const int j = i + 1;

                if (!hasReachedEndOfLine(tokens, j)) {
                    const VJassToken &bracket = tokens.at(j);

                    // comparison
                    if (bracket.getType() == VJassToken::ComparisonOperator && (bracket.getValue() == "==" || bracket.getValue() == "!=")) {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        if (bracket.getValue() == "==") {
                            operation->setType(VJassExpression::Equals);
                        } else if (bracket.getValue() == "!=") {
                            operation->setType(VJassExpression::NotEquals);
                        }

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    // concatenation
                    } else if (bracket.getType() == VJassToken::AndKeyword || bracket.getType() == VJassToken::OrKeyword) {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        if (bracket.getType() == VJassToken::AndKeyword) {
                            operation->setType(VJassExpression::And);
                        } else {
                            operation->setType(VJassExpression::Or);
                        }

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    }
                }

                break;
            }
            case VJassToken::NullKeyword: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::Null);
                result->setValue(nextToken.getValue());

                const int j = i + 1;

                if (!hasReachedEndOfLine(tokens, j)) {
                    const VJassToken &bracket = tokens.at(j);

                    // comparison
                    if (bracket.getType() == VJassToken::ComparisonOperator && (bracket.getValue() == "==" || bracket.getValue() == "!=")) {
                        VJassExpression *operation = new VJassExpression(nextToken.getLine(), nextToken.getColumn());

                        if (bracket.getValue() == "==") {
                            operation->setType(VJassExpression::Equals);
                        } else if (bracket.getValue() == "!=") {
                            operation->setType(VJassExpression::NotEquals);
                        }

                        i++;
                        VJassExpression *rightOperation = parseExpression(tokens, token, ast, i);

                        if (rightOperation != nullptr) {
                            operation->addChild(rightOperation);
                        }

                        result = operation;
                    }
                }

                break;
            }
            case VJassToken::NotKeyword: {
                result = new VJassExpression(nextToken.getLine(), nextToken.getColumn());
                result->setType(VJassExpression::Not);
                result->setValue(nextToken.getValue());

                i++;
                VJassExpression *rightExpression = parseExpression(tokens, token, ast, i);

                if (rightExpression != nullptr) {
                    result->addChild(rightExpression);
                }

                break;
            }
            default: {
                ast->addError(nextToken, "Invalid expression: " + nextToken.getValue());

                break;
            }
        }
    }

    return result;
}

inline VJassGlobal* parseGlobal(bool isConstant, int line, int column, const VJassToken &type, const QList<VJassToken> &tokens, VJassAst *ast, int &i, bool &wasLineBreak) {
    if (!type.isValidType()) {
        ast->addError(type, QObject::tr("Invalid type of global: %1.").arg(type.getValue()));
    }

    i++;

    if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
        if (isConstant) {
            ast->addErrorAtEndOf(type, QObject::tr("Missing identifier of global variable."));
        } else {
            ast->addErrorAtEndOf(type, QObject::tr("Missing array keyword or name of global variable."));
        }
    } else {
        VJassGlobal *global = new VJassGlobal(line, column);
        global->setIsConstant(isConstant);
        global->setType(type.getValue());

        const VJassToken &arrayToken = tokens.at(i);

        if (arrayToken.getType() == VJassToken::ArrayKeyword) {
            global->setIsArray(true);
            i++;
        }

        if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
            ast->addErrorAtEndOf(arrayToken, QObject::tr("Missing identifier of global variable."));
        } else {
            const VJassToken &nameToken = tokens.at(i);

            if (!nameToken.isValidIdentifier()) {
                ast->addError(nameToken, QObject::tr("Invalid identifier for global %1").arg(nameToken.getValue()));
            } else {
                global->setName(nameToken.getValue());
            }

            i++;

            // has assignment
            if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                if (global->getIsConstant()) {
                    global->addErrorAtEndOf(nameToken, QObject::tr("Missing assignment of constant."));
                }
            } else {
                const VJassToken &assignmentOperatorToken = tokens.at(i);

                if (assignmentOperatorToken.getType() != VJassToken::AssignmentOperator) {
                    global->addErrorAtEndOf(assignmentOperatorToken, QObject::tr("Expected assignment operator instead of %1").arg(assignmentOperatorToken.getValue()));
                }

                if (global->getIsArray()) {
                    global->addErrorAtEndOf(assignmentOperatorToken, QObject::tr("Assignments of global array variables are not allowed."));
                } else {
                    VJassExpression *expression = parseExpression(tokens, assignmentOperatorToken, global, i);

                    if (expression != nullptr) {
                        global->addChild(expression);
                    }

                    hasReachedEndOfLine(tokens, i, wasLineBreak); // check for the line break
                }
            }
        }

        return global;
    }

    return nullptr;
}

}

VJassAst* VJassParser::parse(const QList<VJassToken> &tokens) {
    VJassAst *ast = new VJassAst(0, 0);
    bool isInFunction = false;
    bool afterLocalsInFunction = false;
    QStack<VJassStatement*> ifStatements;
    QStack<VJassStatement*> loopStatements;
    VJassFunction *currentFunction = nullptr;
    bool isInGlobals = false;
    VJassGlobals *currentGlobals = nullptr;

    for (int i = 0; i < tokens.size(); i++) {
        const VJassToken &token = tokens.at(i);
        bool wasLineBreak = false;

        switch (token.getType()) {
            case VJassToken::LineBreak: {
                // do nothing, just consume them
                wasLineBreak = true;

                break;
            }
            case VJassToken::TypeKeyword: {
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
                        vjassType->setIdentifier(typeName.getValue());

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
                                    } else {
                                        vjassType->setParent(parentType.getValue());
                                    }
                                }
                            }
                        }

                    } else {
                        vjassType->addError(typeName, "Invalid type identifier: " + typeName.getValue());
                        // TODO add code completion suggestion replace typeName with valid identifier
                    }
                }

                ast->addChild(vjassType);

                break;
            }
            case VJassToken::ConstantKeyword: {
                if (isInGlobals) {
                    i++;

                    if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                        ast->addErrorAtEndOf(token, QObject::tr("Missing type after constant keyword."));
                    } else {
                        const VJassToken &type = tokens.at(i);

                        VJassGlobal *global = parseGlobal(true, token.getLine(), token.getColumn(), type, tokens, ast, i, wasLineBreak);

                        if (global != nullptr) {
                            currentGlobals->addChild(global);
                        }
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

                break;
            }
            case VJassToken::NativeKeyword: {
                VJassNative *vjassNative = new VJassNative(token.getLine(), token.getColumn());

                if (isInFunction) {
                    vjassNative->addError(token, QObject::tr("Cannot declare native inside of function."));
                } else if (isInGlobals) {
                    vjassNative->addError(token, QObject::tr("Cannot declare native inside of globals."));
                }

                parseFunctionDeclaration(tokens, token, vjassNative, ast, i);

                ast->addChild(vjassNative);

                break;
            }
            case VJassToken::GlobalsKeyword: {
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

                break;
            }
            case VJassToken::EndglobalsKeyword: {
                if (!isInGlobals) {
                    ast->addError(token, QObject::tr("Unable to close globals when no globals were declared."));
                }

                isInGlobals = false;
                currentGlobals = nullptr;

                break;
            }
            case VJassToken::FunctionKeyword: {
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

                break;
            }
            case VJassToken::EndfunctionKeyword: {
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

                break;
            }
            case VJassToken::LocalKeyword: {
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

                                            // since the expression consumes all brackets it might now be at the next line
                                            if (i < tokens.size() && tokens.at(i).getType() == VJassToken::LineBreak) {
                                                wasLineBreak = true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    currentFunction->addChild(localStatement);
                }

                break;
            }
            case VJassToken::SetKeyword: {
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

                        if (!variableName.isValidIdentifier()) {
                            ast->addError(variableName, QObject::tr("Invalid variable name %1").arg(variableName.getValue()));
                        } else {
                            const int j = i + 1;

                            if (hasReachedEndOfLine(tokens, j, wasLineBreak)) {
                                ast->addErrorAtEndOf(variableName, QObject::tr("Expected square brackets for array access or assignment operator."));
                            } else {
                                const VJassToken &arrayIndexOperator = tokens.at(j);

                                if (arrayIndexOperator.getType() == VJassToken::LeftSquareBracket) {
                                    VJassExpression *expression = parseExpression(tokens, variableName, ast, i);

                                    if (expression != nullptr) {
                                        setStatement->addChild(expression);
                                    }
                                } else {
                                    i = j;
                                }

                                if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                                    ast->addErrorAtEndOf(variableName, QObject::tr("Missing assignment operator."));
                                } else {
                                    const VJassToken &assignmentOperator = tokens.at(i);

                                    if (assignmentOperator.getType() != VJassToken::AssignmentOperator) {
                                        ast->addError(assignmentOperator, QObject::tr("Invalid assignment operator of %1").arg(assignmentOperator.getValue()));
                                    } else {
                                        VJassExpression *expression = parseExpression(tokens, assignmentOperator, ast, i);

                                        if (expression != nullptr) {
                                            setStatement->addChild(expression);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    currentFunction->addChild(setStatement);
                }

                break;
            } case VJassToken::LoopKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Keyword loop is only allowed inside of a function."));
                } else {
                    afterLocalsInFunction = true;

                    VJassStatement *loopStatement = new VJassStatement(token.getLine(), token.getColumn(), VJassStatement::Loop);

                    currentFunction->addChild(loopStatement);
                    loopStatements.push_back(loopStatement);
                }

                break;
            }  case VJassToken::ExitwhenKeyword: {
                if (loopStatements.isEmpty()) {
                    ast->addError(token, QObject::tr("Keyword exitwhen is only allowed inside of a loop."));
                } else {
                    VJassStatement *exitwhenStatement = new VJassStatement(token.getLine(), token.getColumn(), VJassStatement::Exitwhen);

                    VJassExpression *expression = parseExpression(tokens, token, ast, i);

                    if (expression != nullptr) {
                        exitwhenStatement->addChild(expression);
                    }

                    VJassStatement *currentLoopStatement = loopStatements.back();
                    currentLoopStatement->addChild(exitwhenStatement);
                }

                break;
            } case VJassToken::EndloopKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Keyword endloop is only allowed inside of a function."));
                } else {
                    afterLocalsInFunction = true;
                }

                if (loopStatements.isEmpty()) {
                    ast->addError(token, QObject::tr("Unexpected endloop keyword"));
                } else {
                    loopStatements.pop_back();
                }

                break;
            } case VJassToken::IfKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Keyword if is only allowed inside of a function."));
                } else {
                    afterLocalsInFunction = true;
                    VJassStatement *ifStatement = new VJassStatement(token.getLine(), token.getColumn(), VJassStatement::If);

                    VJassExpression *expression = parseExpression(tokens, token, ast, i);

                    if (expression != nullptr) {
                        ifStatement->addChild(expression);
                    }

                    if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                        ast->addErrorAtEndOf(token, QObject::tr("Expected then keyword but line ends."));
                    } else {
                        const VJassToken &thenToken = tokens.at(i);

                        if (thenToken.getType() != VJassToken::ThenKeyword) {
                            ast->addErrorAtEndOf(thenToken, QObject::tr("Expected then keyword instead of %1").arg(thenToken.getValue()));
                        }
                    }

                    currentFunction->addChild(ifStatement);
                    ifStatements.push_back(ifStatement);
                }

                break;
            } case VJassToken::ElseifKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Unexpected elseif outside of function body"));
                } else {
                    afterLocalsInFunction = true;
                }

                if (ifStatements.isEmpty()) {
                    ast->addError(token, QObject::tr("Unexpected elseif keyword"));
                } else {
                    VJassStatement *currentIfStatement = ifStatements.back();

                    if (currentIfStatement->getHasElse()) {
                        currentIfStatement->addError(token, QObject::tr("Unexpected elseif keyword after having already one else statement"));
                    } else {
                        VJassStatement *elseifStatement = new VJassStatement(token.getLine(), token.getColumn(), VJassStatement::Elseif);

                        VJassExpression *expression = parseExpression(tokens, token, ast, i);

                        if (expression != nullptr) {
                            elseifStatement->addChild(expression);
                        }

                        if (hasReachedEndOfLine(tokens, i, wasLineBreak)) {
                            ast->addErrorAtEndOf(token, QObject::tr("Expected then keyword."));
                        } else {
                            const VJassToken &thenToken = tokens.at(i);

                            if (thenToken.getType() != VJassToken::ThenKeyword) {
                                ast->addErrorAtEndOf(thenToken, QObject::tr("Expected then keyword instead of %1").arg(thenToken.getValue()));
                            }
                        }

                        currentIfStatement->addChild(elseifStatement);
                    }
                }

                break;
            } case VJassToken::ElseKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Unexpected else outside of function body"));
                } else {
                    afterLocalsInFunction = true;
                }

                if (ifStatements.isEmpty()) {
                    ast->addError(token, QObject::tr("Unexpected else keyword"));
                } else {
                    VJassStatement *currentIfStatement = ifStatements.back();

                    if (currentIfStatement->getHasElse()) {
                        currentIfStatement->addError(token, QObject::tr("Unexpected else keyword after having already one"));
                    } else {
                        VJassStatement *elseStatement = new VJassStatement(token.getLine(), token.getColumn(), VJassStatement::Else);

                        currentIfStatement->setHasElse(true);
                        currentIfStatement->addChild(elseStatement);
                    }
                }

                break;
            } case VJassToken::EndifKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Unexpected endif outside of function body"));
                } else {
                    afterLocalsInFunction = true;
                }

                if (ifStatements.isEmpty()) {
                    ast->addError(token, QObject::tr("Unexpected endif keyword"));
                } else {
                    ifStatements.pop_back();
                }

                break;
            } case VJassToken::CallKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Unexpected call outside of function body"));
                } else {
                    afterLocalsInFunction = true;

                    VJassStatement *callStatement = new VJassStatement(token.getLine(), token.getColumn(), VJassStatement::Call);

                    VJassExpression *expression = parseExpression(tokens, token, ast, i);

                    if (expression != nullptr) {
                        callStatement->addChild(expression);
                    } else {
                        ast->addErrorAtEndOf(token, QObject::tr("Missing call expression."));
                    }

                    currentFunction->addChild(callStatement);
                }

                break;
            } case VJassToken::ReturnKeyword: {
                if (!isInFunction) {
                    ast->addError(token, QObject::tr("Unexpected return outside of function body"));
                } else {
                    afterLocalsInFunction = true;

                    VJassStatement *returnStatement = new VJassStatement(token.getLine(), token.getColumn(), VJassStatement::Return);

                    VJassExpression *expression = parseExpression(tokens, token, ast, i, 0, false);

                    // return expression is optional
                    if (expression != nullptr) {
                        returnStatement->addChild(expression);
                    }

                    currentFunction->addChild(returnStatement);
                }

                break;
            } case VJassToken::Comment: {
                ast->addComment(token.getValue());

                break;
            } case VJassToken::Unknown: {
                ast->addError(token, QObject::tr("Unknown text %1").arg(token.getValue()));

                break;
            } case VJassToken::Text: {
                if (isInGlobals) {
                    VJassGlobal *global = parseGlobal(false, token.getLine(), token.getColumn(), token, tokens, ast, i, wasLineBreak);

                    if (global != nullptr) {
                        currentGlobals->addChild(global);
                    }
                } else {
                    suggestLineStartKeywords(isInFunction, isInGlobals, *ast, &token);

                    ast->addError(token, QObject::tr("Unknown text %1").arg(token.getValue()));
                }

                break;
            // all keywords not handled at this point should be invalid here
            } default: {
                if (token.isValidKeyword()) {
                    ast->addError(token, "Unexpected keyword: " + token.getValue());
                }

                break;
            }
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
