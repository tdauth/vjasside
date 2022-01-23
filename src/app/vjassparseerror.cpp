#include "vjassparseerror.h"

VJassParseError::VJassParseError() : line(0), column(0), length(0) {
}

VJassParseError::VJassParseError(int line, int column, int length, const QString &error) : line(line), column(column), length(length), error(error) {
}

VJassParseError::VJassParseError(const VJassParseError &other) : line(other.getLine()), column(other.getColumn()), length(other.getLength()), error(other.getError()) {
}

VJassParseError& VJassParseError::operator=(const VJassParseError &other) {
    this->line = other.getLine();
    this->column = other.getColumn();
    this->length = other.getLength();
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

int VJassParseError::getLength() const {
    return length;
}
