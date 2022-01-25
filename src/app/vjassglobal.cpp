#include "vjassglobal.h"

VJassGlobal::VJassGlobal(int line, int column) : VJassAst(line, column), isArray(false), isConstant(false)
{

}

void VJassGlobal::setName(const QString &name) {
    this->name = name;
}

void VJassGlobal::setType(const QString &type) {
    this->type = type;
}

void VJassGlobal::setIsArray(bool isArray) {
    this->isArray = isArray;
}

void VJassGlobal::setIsConstant(bool isConstant) {
    this->isConstant = isConstant;
}

bool VJassGlobal::getIsArray() const {
    return isArray;
}

bool VJassGlobal::getIsConstant() const {
    return isConstant;
}
