#include "vjassglobal.h"
#include "vjasstoken.h"

VJassGlobal::VJassGlobal(int line, int column) : VJassAst(line, column), isArray(false), isConstant(false)
{
}

void VJassGlobal::setName(const QString &name) {
    this->name = name;
}

const QString& VJassGlobal::getName() const {
    return name;
}

void VJassGlobal::setType(const QString &type) {
    this->type = type;
}

const QString& VJassGlobal::getType() const {
    return type;
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

QString VJassGlobal::toString() const {
    QString result;

    if (getIsConstant()) {
         result = VJassToken::KEYWORD_CONSTANT + " ";
    }

    result += type + " ";


    if (getIsArray()) {
        result += "array ";
    }

    result += name + " ";

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


    return result;
}
