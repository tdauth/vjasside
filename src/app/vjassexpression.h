#ifndef VJASSEXPRESSION_H
#define VJASSEXPRESSION_H

#include "vjassast.h"

class VJassExpression : public VJassAst
{
public:
    VJassExpression(int line, int column);

    enum Type {
        StringLiteral,
        IntegerLiteral,
        RealLiteral,
        True,
        False,
        FunctionCall,
        Brackets,
        Identifier,
        And,
        Or,
        Not,
        Positive,
        Negative,
        Sum,
        Substraction,
        Multiplication,
        Division
    };

    void setValue(const QString &value);
    QString getValue() const;

    void setType(Type type);
    Type getType() const;

    // TODO Function parameters would be the children

private:
    QString value;
    Type type;
};

#endif // VJASSEXPRESSION_H
