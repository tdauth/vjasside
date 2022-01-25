#ifndef VJASSGLOBAL_H
#define VJASSGLOBAL_H

#include "vjassast.h"

class VJassGlobal : public VJassAst
{
public:
    VJassGlobal(int line, int column);

    void setName(const QString &name);
    void setType(const QString &type);
    void setIsArray(bool isArray);
    void setIsConstant(bool isConstant);
    bool getIsArray() const;
    bool getIsConstant() const;

private:
    QString name;
    QString type;
    bool isArray;
    bool isConstant;
};

#endif // VJASSGLOBAL_H
