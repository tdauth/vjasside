#ifndef VJASSPARSER_H
#define VJASSPARSER_H

#include <QList>

#include "vjassast.h"


class VJassParser
{
public:
    VJassParser();

    VJassAst parse(QString content,  QList<VJassToken> &tokens);
};

#endif // VJASSPARSER_H
