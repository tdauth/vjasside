#ifndef VJASSCODEELEMENT_H
#define VJASSCODEELEMENT_H


class VJassCodeElement
{
public:
    VJassCodeElement();
    virtual ~VJassCodeElement();

    virtual int getColumn() const = 0;
    virtual int getLine() const = 0;
    virtual int getLength() const = 0;
    virtual bool isSyntaxError() const = 0;
    virtual bool isToken() const = 0;
    virtual bool highlight() const = 0;
};

#endif // VJASSCODEELEMENT_H
