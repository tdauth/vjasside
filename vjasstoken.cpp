#include <QtCore>

#include "vjasstoken.h"

const QString VJassToken::KEYWORD_FUNCTION = "function";
const QString VJassToken::KEYWORD_TAKES = "takes";
const QString VJassToken::KEYWORD_NOTHING = "nothing";
const QString VJassToken::KEYWORD_RETURNS = "returns";
const QString VJassToken::KEYWORD_ENDFUNCTION = "endfunction";
const QString VJassToken::KEYWORD_GLOBALS = "globals";
const QString VJassToken::KEYWORD_CONSTANT = "constant";
const QString VJassToken::KEYWORD_TYPE = "type";
const QString VJassToken::KEYWORD_NATIVE = "native";

const QStringList VJassToken::KEYWRODS_ALL = {
    VJassToken::KEYWORD_FUNCTION,
    VJassToken::KEYWORD_TAKES,
    VJassToken::KEYWORD_NOTHING,
    VJassToken::KEYWORD_RETURNS,
    VJassToken::KEYWORD_ENDFUNCTION,
    VJassToken::KEYWORD_GLOBALS,
    VJassToken::KEYWORD_CONSTANT,
    VJassToken::KEYWORD_TYPE,
    VJassToken::KEYWORD_NATIVE
};

VJassToken::VJassToken(const QString &value, int line, int column, Type type) : value(value), line(line), column(column), type(type)
{
}

const QString& VJassToken::getValue() const {
    return value;
}

int VJassToken::getLine() const {
    return line;
}

int VJassToken::getColumn() const {
    return column;
}

VJassToken::Type VJassToken::getType() const {
    return type;
}

bool VJassToken::isValidType() const {
    QSet<QString> standardTypes;
    // TODO parse natives instead of adding them here
    standardTypes.insert("integer");
    standardTypes.insert("real");
    standardTypes.insert("boolean");
    standardTypes.insert("unit");

    return standardTypes.contains(getValue());
}

bool VJassToken::isValidIdentifier() const {
    return QRegularExpression("[a-zA-Z0-9]+").match(getValue()).hasMatch() && !VJassToken::KEYWRODS_ALL.contains(getValue());
}

bool VJassToken::isValidKeyword() const {
    return KEYWRODS_ALL.contains(getValue());
}

VJassToken::Type VJassToken::typeFromKeyword(const QString &keyword) {
    if (keyword == KEYWORD_FUNCTION) {
        return VJassToken::FunctionKeyword;
    } else if (keyword == KEYWORD_TAKES) {
        return VJassToken::TakesKeyword;
    } else if (keyword == KEYWORD_NOTHING) {
        return VJassToken::NothingKeyword;
    } else if (keyword == KEYWORD_RETURNS) {
        return VJassToken::ReturnsKeyword;
    }

    return VJassToken::FunctionKeyword;
}
