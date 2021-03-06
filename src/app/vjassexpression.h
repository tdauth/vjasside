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
        RawCodeLiteral,
        True,
        False,
        Null,
        FunctionCall,
        ArrayAccess,
        Brackets,
        Identifier,
        And,
        Or,
        Not,
        Negative,
        Sum,
        Substraction,
        Multiplication,
        Division,
        Equals,
        NotEquals,
        GreaterThan,
        LessThan,
        LessThanOrEquals,
        GreaterThanOrEquals
    };

    void setValue(const QString &value);
    const QString& getValue() const;

    void setType(Type type);
    Type getType() const;

    // TODO Function parameters would be the children

    virtual QString toString() const override;

private:
    QString value;
    Type type;
};

#endif // VJASSEXPRESSION_H
