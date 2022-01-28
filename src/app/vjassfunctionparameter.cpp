#include "vjassfunctionparameter.h"

VJassFunctionParameter::VJassFunctionParameter(int line, int column, const QString &type, const QString &name) : VJassAst(line, column), type(type), name(name)
{

}

QString VJassFunctionParameter::toString() const {
    return type + " " + name;
}
