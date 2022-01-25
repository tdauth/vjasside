#include "vjassexpression.h"

VJassExpression::VJassExpression(int line, int column) : VJassAst(line, column)
{
}

void VJassExpression::setValue(const QString &value) {
    this->value = value;
}

QString VJassExpression::getValue() const {
    return value;
}

void VJassExpression::setType(VJassExpression::Type type) {
    this->type = type;
}

VJassExpression::Type VJassExpression::getType() const {
    return type;
}
