#ifndef VJASSGLOBAL_H
#define VJASSGLOBAL_H

#include "vjassast.h"

class VJassGlobal : public VJassAst
{
public:
    VJassGlobal(int line, int column);

    void setName(const QString &name);
    const QString& getName() const;
    void setType(const QString &type);
    const QString& getType() const;
    void setIsArray(bool isArray);
    void setIsConstant(bool isConstant);
    bool getIsArray() const;
    bool getIsConstant() const;

    virtual QString toString() const override;

private:
    QString name;
    QString type;
    bool isArray;
    bool isConstant;
};

#endif // VJASSGLOBAL_H
