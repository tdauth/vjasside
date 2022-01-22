#ifndef VJASSFUNCTIONPARAMETER_H
#define VJASSFUNCTIONPARAMETER_H

#include <QString>

class VJassFunctionParameter
{
public:
    VJassFunctionParameter(const QString &type, const QString &name);

private:
    QString type;
    QString name;
};

#endif // VJASSFUNCTIONPARAMETER_H
