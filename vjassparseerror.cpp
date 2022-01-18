#include "vjassparseerror.h"

VJassParseError::VJassParseError(int line, int column, const QString &error) : line(line), column(column), error(error)
{

}

int VJassParseError::getLine() {
    return line;
}

int VJassParseError::getColumn() {
    return column;
}

const QString& VJassParseError::getError() {
    return error;
}
