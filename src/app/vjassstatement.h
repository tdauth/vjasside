#ifndef VJASSSTATEMENT_H
#define VJASSSTATEMENT_H

#include "vjassast.h"

class VJassStatement : public VJassAst
{
public:
    VJassStatement(int line, int column);

    enum Type {
        Local,
        Set,
        Call,
        If,
        Elseif,
        Endif,
        Loop,
        Exitwhen,
        Endloop
    };

    void setType(Type type);
    Type getType() const;

private:
    Type type;
};

#endif // VJASSSTATEMENT_H
