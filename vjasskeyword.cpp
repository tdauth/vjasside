#include "vjasskeyword.h"

VJassKeyword::VJassKeyword(int line, int column) : VJassAst(line, column)
{

}

void VJassKeyword::setKeyword(const QString &keyword) {
    this->keyword = keyword;
}

const QString& VJassKeyword::getKeyword() const {
    return keyword;
}

QString VJassKeyword::toString() {
    return getKeyword();
}
