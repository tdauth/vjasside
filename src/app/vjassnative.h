#ifndef VJASSNATIVE_H
#define VJASSNATIVE_H

#include "vjassast.h"
#include "vjassfunctionparameter.h"


class VJassNative: public VJassAst
{
public:
    VJassNative(int line, int column);

    void setIdentifier(const QString &identifier);
    void addParameter(int line, int column, const QString &type, const QString &name);
    void setReturnType(const QString &returnType);

    virtual QString toString() const override;

private:
    QString identifier;
    QList<VJassFunctionParameter> parameters;
    QString returnType;
};

#endif // VJASSNATIVE_H
