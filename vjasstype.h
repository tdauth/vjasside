#ifndef VJASSTYPE_H
#define VJASSTYPE_H

#include "vjassast.h"

class VJassType: public VJassAst
{
public:
    VJassType(int line, int column);

    void setIdentifier(const QString &identifier);
    void setParent(const QString &parent);

private:
    QString identifier;
    QString parent;
};

#endif // VJASSTYPE_H
