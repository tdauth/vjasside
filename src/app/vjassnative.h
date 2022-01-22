#ifndef VJASSNATIVE_H
#define VJASSNATIVE_H

#include "vjassast.h"
#include "vjassfunctionparameter.h"


class VJassNative: public VJassAst
{
public:
    VJassNative(int line, int column);

    void setIdentifier(const QString &identifier);
    void addParameter(const QString &type, const QString &name);
    void setReturnType(const QString &returnType);

private:
    QString identifier;
    QList<VJassFunctionParameter> parameters;
    QString returnType;
};

#endif // VJASSNATIVE_H
