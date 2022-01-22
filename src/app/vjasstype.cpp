#include "vjasstype.h"

VJassType::VJassType(int line, int column) : VJassAst(line, column)
{
}

VJassType::VJassType(const VJassType &other)
    : VJassAst(other)
    , identifier(other.getIdentifier())
    , parent(other.getParent())
{
}

VJassType& VJassType::operator=(const VJassType &other) {
    VJassAst::operator=(other);

    setIdentifier(other.getIdentifier());
    setParent(other.getParent());

    return *this;
}

VJassType::~VJassType() {
}

const QString& VJassType::getIdentifier() const {
    return identifier;
}

void VJassType::setIdentifier(const QString &identifier) {
    this->identifier = identifier;
}

const QString& VJassType::getParent() const {
    return parent;
}

void VJassType::setParent(const QString &parent) {
    this->parent = parent;
}
