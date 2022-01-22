#ifndef VJASSFUNCTION_H
#define VJASSFUNCTION_H

#include "vjassast.h"
#include "vjassfunctionparameter.h"


class VJassFunction : public VJassAst
{
public:
    VJassFunction(int line, int column);

    void setIdentifier(const QString &identifier);
    void addParameter(const QString &type, const QString &name);
    void setReturnType(const QString &returnType);

private:
    QString identifier;
    QList<VJassFunctionParameter> parameters;
    QString returnType;
};

#endif // VJASSFUNCTION_H
