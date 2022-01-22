#include "vjassnative.h"

VJassNative::VJassNative(int line, int column) : VJassAst(line, column)
{
}

void VJassNative::setIdentifier(const QString &identifier) {
    this->identifier = identifier;
}

void VJassNative::addParameter(const QString &type, const QString &name) {
    parameters.append(VJassFunctionParameter(type, name));
}

void VJassNative::setReturnType(const QString &returnType) {
    this->returnType = returnType;
}
