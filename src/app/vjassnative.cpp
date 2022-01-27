#include "vjassnative.h"
#include "vjasstoken.h"

VJassNative::VJassNative(int line, int column) : VJassAst(line, column)
{
}

void VJassNative::setIdentifier(const QString &identifier) {
    this->identifier = identifier;
}

void VJassNative::addParameter(int line, int column, const QString &type, const QString &name) {
    parameters.append(VJassFunctionParameter(line, column, type, name));
}

void VJassNative::setReturnType(const QString &returnType) {
    this->returnType = returnType;
}

QString VJassNative::toString() const {
    QString result = VJassToken::KEYWORD_NATIVE + " " + identifier + " " + VJassToken::KEYWORD_TAKES + " ";

    if (parameters.isEmpty()) {
        result += VJassToken::KEYWORD_NOTHING;
    } else {
        int i = 0;

        for (VJassFunctionParameter p : parameters) {
            result += p.toString();

            if (i < parameters.size() - 1) {
                result += ", ";
            }

            i++;
        }
    }

    result += " returns " + returnType;

    return result;
}
