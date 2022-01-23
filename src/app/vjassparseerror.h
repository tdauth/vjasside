#ifndef VJASSPARSEERROR_H
#define VJASSPARSEERROR_H

#include <QString>

class VJassParseError
{
public:
    VJassParseError();
    // TODO handle errors over multiple lines
    VJassParseError(int line, int column, int length, const QString &error);
    VJassParseError(const VJassParseError &other);
    VJassParseError& operator=(const VJassParseError &other);

    int getLine() const;
    int getColumn() const;
    const QString& getError() const;

    int getLength() const;

private:
    int line;
    int column;
    int length;
    QString error;
};

#endif // VJASSPARSEERROR_H
