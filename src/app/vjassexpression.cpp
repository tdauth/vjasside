#include "vjassexpression.h"

VJassExpression::VJassExpression(int line, int column) : VJassAst(line, column)
{
}

void VJassExpression::setValue(const QString &value) {
    this->value = value;
}

const QString& VJassExpression::getValue() const {
    return value;
}

void VJassExpression::setType(VJassExpression::Type type) {
    this->type = type;
}

VJassExpression::Type VJassExpression::getType() const {
    return type;
}

QString VJassExpression::toString() const {
    QString result;

    switch (getType()) {
        case VJassExpression::Brackets: {
            result = "(";

            if (!getChildren().isEmpty()) {
                int i = 0;

                for (VJassAst *child : getChildren()) {
                    result += child->toString();

                    if (i < getChildren().size() - 1) {
                        result += ", ";
                    }

                    i++;
                }
            }

            result += ")";

            break;
        }

        case VJassExpression::True: {
            return VJassToken::KEYWORD_TRUE;
        }

        case VJassExpression::False: {
            return VJassToken::KEYWORD_FALSE;
        }

        case VJassExpression::Not: {
            result = VJassToken::KEYWORD_NOT;

            if (!getChildren().isEmpty()) {
                result += " ";
                int i = 0;

                for (VJassAst *child : getChildren()) {
                    result += child->toString();

                    if (i < getChildren().size() - 1) {
                        result += ", ";
                    }

                    i++;
                }
            }

            break;
        }

        case VJassExpression::FunctionCall: {
            result = getValue();

            if (!getChildren().isEmpty()) {
                int i = 0;

                for (VJassAst *child : getChildren()) {
                    result += child->toString();

                    if (i < getChildren().size() - 1) {
                        result += ", ";
                    }

                    i++;
                }
            }

            break;
        }

        case VJassExpression::Identifier:
        case VJassExpression::StringLiteral:
        case VJassExpression::RawCodeLiteral:
        case VJassExpression::IntegerLiteral:
        case VJassExpression::RealLiteral: {
            result = getValue();

            break;
        }

        default: {
            result = "expression ";

            switch (getType()) {
                case VJassExpression::And: {
                    result += "and";

                    break;
                }

                case VJassExpression::Or: {
                    result += "or";

                    break;
                }

                case VJassExpression::Division: {
                    result += "/";

                    break;
                }
            }

            result += " with children";

            if (!getChildren().isEmpty()) {
                result += "(";

                int i = 0;

                for (VJassAst *child : getChildren()) {
                    result += child->toString();

                    if (i < getChildren().size() - 1) {
                        result += ", ";
                    }

                    i++;
                }

                result += ")";
            }
        }
    }

    return result;
}
