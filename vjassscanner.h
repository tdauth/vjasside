#ifndef VJASSSCANNER_H
#define VJASSSCANNER_H

#include <QList>

#include "vjasstoken.h"


class VJassScanner
{
public:
    VJassScanner();

    QList<VJassToken> scan(const QString &content, bool dropWhiteSpaces = true);
};

#endif // VJASSSCANNER_H
