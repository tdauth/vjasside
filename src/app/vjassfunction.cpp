#include "vjassfunction.h"
#include "vjasstoken.h"

VJassFunction::VJassFunction(int line, int column) : VJassNative(line, column)
{
}

QString VJassFunction::toString() const {
    QString result = VJassToken::KEYWORD_FUNCTION + " " + getIdentifier() + " " + VJassToken::KEYWORD_TAKES + " ";

    if (getParameters().isEmpty()) {
        result += VJassToken::KEYWORD_NOTHING;
    } else {
        int i = 0;

        for (const VJassFunctionParameter &p : getParameters()) {
            result += p.toString();

            if (i < getParameters().size() - 1) {
                result += ", ";
            }

            i++;
        }
    }

    result += " returns " + getReturnType();

    return result;
}
