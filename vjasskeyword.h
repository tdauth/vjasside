#ifndef VJASSKEYWORD_H
#define VJASSKEYWORD_H

#include "vjassast.h"

class VJassKeyword : public VJassAst
{
public:
    VJassKeyword(int line, int column);

    void setKeyword(const QString &keyword);
    const QString& getKeyword() const;

    QString toString() override;

private:
    QString keyword;
};

#endif // VJASSKEYWORD_H
