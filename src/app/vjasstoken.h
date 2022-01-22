#ifndef VJASSTOKEN_H
#define VJASSTOKEN_H

#include <QString>
#include <QStringList>

class VJassToken
{
public:
    const static QString KEYWORD_ENDFUNCTION; // define before function to match first
    const static QString KEYWORD_FUNCTION;
    const static QString KEYWORD_TAKES;
    const static QString KEYWORD_NOTHING;
    const static QString KEYWORD_RETURNS;
    const static QString KEYWORD_GLOBALS;
    const static QString KEYWORD_ENDGLOBALS;
    const static QString KEYWORD_CONSTANT;
    const static QString KEYWORD_TYPE;
    const static QString KEYWORD_EXTENDS;
    const static QString KEYWORD_NATIVE;
    const static QString KEYWORD_NULL;
    const static QStringList KEYWRODS_ALL;

    // keep Warcraft III's default stuff in memory to improve the performance on highlighting
    const static QSet<QString> COMMONJ_TYPES_ALL;
    const static QSet<QString> COMMONJ_NATIVES_ALL;
    const static QSet<QString> COMMONJ_CONSTANTS_ALL;

    enum Type {
        FunctionKeyword,
        TakesKeyword,
        NothingKeyword,
        ReturnsKeyword,
        EndfunctionKeyword,
        ConstantKeyword,
        TypeKeyword,
        ExtendsKeyword,
        NativeKeyword,
        GlobalsKeyword,
        EndglobalsKeyword,
        NullKeyword,
        Comment,
        Operator,
        BooleanLiteral,
        IntegerLiteral,
        RealLiteral,
        RawCodeLiteral,
        LineBreak,
        WhiteSpace,
        Separator,
        Text,
        Unknown
    };

    VJassToken(const QString &value, int line, int column, Type type);

    const QString& getValue() const;
    int getLine() const;
    int getColumn() const;
    Type getType() const;

    bool isValidType() const;
    bool isValidIdentifier() const;
    bool isValidKeyword() const;

    static VJassToken::Type typeFromKeyword(const QString &keyword);

    int getValueLength() const;
    bool isCommonJType() const;
    bool isCommonJNative() const;
    bool isCommonJConstant() const;

private:
    QString value;
    int line;
    int column;
    Type type;

    // cached stuff for faster access
    int valueLengthCached;
    bool isCommonJTypeCached;
    bool isCommonJNativeCached;
    bool isCommonJConstantCached;
};

#endif // VJASSTOKEN_H
