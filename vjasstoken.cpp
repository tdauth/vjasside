#include <exception>

#include <QtCore>

#include "vjasstoken.h"

const QString VJassToken::KEYWORD_ENDFUNCTION = "endfunction";
const QString VJassToken::KEYWORD_FUNCTION = "function";
const QString VJassToken::KEYWORD_TAKES = "takes";
const QString VJassToken::KEYWORD_NOTHING = "nothing";
const QString VJassToken::KEYWORD_RETURNS = "returns";
const QString VJassToken::KEYWORD_GLOBALS = "globals";
const QString VJassToken::KEYWORD_ENDGLOBALS = "endglobals";
const QString VJassToken::KEYWORD_CONSTANT = "constant";
const QString VJassToken::KEYWORD_TYPE = "type";
const QString VJassToken::KEYWORD_EXTENDS = "extends";
const QString VJassToken::KEYWORD_NATIVE = "native";

const QStringList VJassToken::KEYWRODS_ALL = {
    VJassToken::KEYWORD_ENDFUNCTION, // match before function
    VJassToken::KEYWORD_FUNCTION,
    VJassToken::KEYWORD_TAKES,
    VJassToken::KEYWORD_NOTHING,
    VJassToken::KEYWORD_RETURNS,
    VJassToken::KEYWORD_GLOBALS,
    VJassToken::KEYWORD_ENDGLOBALS,
    VJassToken::KEYWORD_CONSTANT,
    VJassToken::KEYWORD_TYPE,
    VJassToken::KEYWORD_EXTENDS,
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
    return getType() == VJassToken::EndfunctionKeyword
        || getType() == VJassToken::FunctionKeyword
        || getType() == VJassToken::TakesKeyword
        || getType() == VJassToken::NothingKeyword
        || getType() == VJassToken::ReturnsKeyword
        || getType() == VJassToken::ConstantKeyword
        || getType() == VJassToken::TypeKeyword
        || getType() == VJassToken::ExtendsKeyword
        || getType() == VJassToken::NativeKeyword
        || getType() == VJassToken::GlobalsKeyword
        || getType() == VJassToken::EndglobalsKeyword
    ;
}

VJassToken::Type VJassToken::typeFromKeyword(const QString &keyword) {
    if (keyword == KEYWORD_ENDFUNCTION) {
        return VJassToken::EndfunctionKeyword;
    } else if (keyword == KEYWORD_FUNCTION) {
        return VJassToken::FunctionKeyword;
    } else if (keyword == KEYWORD_TAKES) {
        return VJassToken::TakesKeyword;
    } else if (keyword == KEYWORD_NOTHING) {
        return VJassToken::NothingKeyword;
    } else if (keyword == KEYWORD_RETURNS) {
        return VJassToken::ReturnsKeyword;
    } else if (keyword == KEYWORD_CONSTANT) {
        return VJassToken::ConstantKeyword;
    } else if (keyword == KEYWORD_TYPE) {
        return VJassToken::TypeKeyword;
    } else if (keyword == KEYWORD_EXTENDS) {
        return VJassToken::ExtendsKeyword;
    } else if (keyword == KEYWORD_NATIVE) {
        return VJassToken::NativeKeyword;
    } else if (keyword == KEYWORD_GLOBALS) {
        return VJassToken::GlobalsKeyword;
    } else if (keyword == KEYWORD_ENDGLOBALS) {
        return VJassToken::EndglobalsKeyword;
    }

    Q_ASSERT(false);
}
