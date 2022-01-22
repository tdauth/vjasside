#include "vjassfunction.h"

VJassFunction::VJassFunction(int line, int column) : VJassAst(line, column)
{
}

void VJassFunction::setIdentifier(const QString &identifier) {
    this->identifier = identifier;
}

void VJassFunction::addParameter(const QString &type, const QString &name) {
    parameters.append(VJassFunctionParameter(type, name));
}

void VJassFunction::setReturnType(const QString &returnType) {
    this->returnType = returnType;

}
