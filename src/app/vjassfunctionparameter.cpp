#include "vjassfunctionparameter.h"

VJassFunctionParameter::VJassFunctionParameter(int line, int column, const QString &type, const QString &name) : type(type), name(name), VJassAst(line, column)
{

}

QString VJassFunctionParameter::toString() const {
    return type + " " + name;
}
