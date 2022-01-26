#ifndef VJASSLOCALSTATEMENT_H
#define VJASSLOCALSTATEMENT_H

#include "vjassast.h"

class VJassLocalStatement : public VJassAst
{
public:
    VJassLocalStatement(int line, int column);

    void setType(const QString &type);
    const QString& getType() const;

    void setVariableName(const QString &variableName);
    const QString& getVariableName() const;

private:
    QString type;
    QString variableName;
};

#endif // VJASSLOCALSTATEMENT_H
