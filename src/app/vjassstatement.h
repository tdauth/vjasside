#ifndef VJASSSTATEMENT_H
#define VJASSSTATEMENT_H

#include "vjassast.h"

class VJassStatement : public VJassAst
{
public:
    enum Type {
        Local,
        Set,
        Call,
        If,
        Elseif,
        Else,
        Endif,
        Loop,
        Exitwhen,
        Endloop,
        Return
    };

    VJassStatement(int line, int column, Type type);

    Type getType() const;

    void setHasElse(bool hasElse);
    bool getHasElse() const;

private:
    const Type type;
    bool hasElse; // for if statements
};

#endif // VJASSSTATEMENT_H
