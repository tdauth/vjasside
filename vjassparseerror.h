#ifndef VJASSPARSEERROR_H
#define VJASSPARSEERROR_H

#include <QString>


class VJassParseError
{
public:
    VJassParseError(int line, int column, const QString &error);

    int getLine();
    int getColumn();
    const QString& getError();

private:
    int line;
    int column;
    QString error;
};

#endif // VJASSPARSEERROR_H
