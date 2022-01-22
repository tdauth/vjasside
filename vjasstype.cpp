#include "vjasstype.h"

VJassType::VJassType(int line, int column) : VJassAst(line, column)
{
}

void VJassType::setIdentifier(const QString &identifier) {
    this->identifier = identifier;
}

void VJassType::setParent(const QString &parent) {
    this->parent = parent;
}
