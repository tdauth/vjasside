#ifndef VJASSTYPE_H
#define VJASSTYPE_H

#include "vjassast.h"

class VJassType: public VJassAst
{
public:
    VJassType(int line, int column);
    VJassType(const VJassType &other);
    VJassType& operator=(const VJassType &other);
    virtual ~VJassType();

    const QString& getIdentifier() const;
    void setIdentifier(const QString &identifier);
    const QString& getParent() const;
    void setParent(const QString &parent);

private:
    QString identifier;
    QString parent;
};

#endif // VJASSTYPE_H
