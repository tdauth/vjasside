#include "vjassparseerror.h"

VJassParseError::VJassParseError() : line(0), column(0) {
}

VJassParseError::VJassParseError(int line, int column, const QString &error) : line(line), column(column), error(error) {
}

VJassParseError::VJassParseError(const VJassParseError &other) : line(other.getLine()), column(other.getColumn()), error(other.getError()) {
}

VJassParseError& VJassParseError::operator=(const VJassParseError &other) {
    this->line = other.getLine();
    this->column = other.getColumn();
    this->error = other.getError();

    return *this;
}

int VJassParseError::getLine() const {
    return line;
}

int VJassParseError::getColumn() const {
    return column;
}

const QString& VJassParseError::getError() const {
    return error;
}
