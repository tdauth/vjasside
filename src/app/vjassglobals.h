#ifndef VJASSGLOBALS_H
#define VJASSGLOBALS_H

#include "vjassast.h"


class VJassGlobals: public VJassAst
{
public:
    VJassGlobals(int line, int column);
    virtual ~VJassGlobals();

    virtual QString toString() const override;
};

#endif // VJASSGLOBALS_H
