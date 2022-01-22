#ifndef VJASSPARSEERROR_H
#define VJASSPARSEERROR_H

#include <QString>


class VJassParseError
{
public:
    VJassParseError();
    VJassParseError(int line, int column, const QString &error);
    VJassParseError(const VJassParseError &other);
    VJassParseError& operator=(const VJassParseError &other);

    int getLine() const;
    int getColumn() const;
    const QString& getError() const;

private:
    int line;
    int column;
    QString error;
};

#endif // VJASSPARSEERROR_H
