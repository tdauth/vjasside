#ifndef VJASSNATIVE_H
#define VJASSNATIVE_H

#include "vjassast.h"
#include "vjassfunctionparameter.h"

class VJassNative: public VJassAst
{
public:
    using Parameters = QList<VJassFunctionParameter>;

    VJassNative(int line, int column);

    void setIdentifier(const QString &identifier);
    const QString& getIdentifier() const;
    void addParameter(int line, int column, const QString &type, const QString &name);
    const Parameters& getParameters() const;
    void setReturnType(const QString &returnType);
    const QString& getReturnType() const;

    virtual QString toString() const override;

private:
    QString identifier;
    Parameters parameters;
    QString returnType;
};

#endif // VJASSNATIVE_H
