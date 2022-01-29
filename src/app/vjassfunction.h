#ifndef VJASSFUNCTION_H
#define VJASSFUNCTION_H

#include "vjassnative.h"


class VJassFunction : public VJassNative
{
public:
    VJassFunction(int line, int column);

    virtual QString toString() const override;
};

#endif // VJASSFUNCTION_H
