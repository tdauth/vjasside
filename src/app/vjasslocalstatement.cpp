#include "vjasslocalstatement.h"

VJassLocalStatement::VJassLocalStatement(int line, int column) : VJassAst(line, column)
{

}

void VJassLocalStatement::setType(const QString &type) {
    this->type = type;
}

const QString& VJassLocalStatement::getType() const {
    return type;
}

void VJassLocalStatement::setVariableName(const QString &variableName) {
    this->variableName = variableName;
}

const QString& VJassLocalStatement::getVariableName() const {
    return variableName;
}
