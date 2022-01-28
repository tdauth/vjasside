#ifndef VJASSTOKEN_H
#define VJASSTOKEN_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>

class VJassToken
{
public:
    const static QString KEYWORD_ENDFUNCTION; // define before function to match first
    const static QString KEYWORD_FUNCTION;
    const static QString KEYWORD_TAKES;
    const static QString KEYWORD_NOTHING;
    const static QString KEYWORD_RETURNS;
    const static QString KEYWORD_RETURN;
    const static QString KEYWORD_LOCAL;
    const static QString KEYWORD_SET;
    const static QString KEYWORD_CALL;
    const static QString KEYWORD_IF;
    const static QString KEYWORD_THEN;
    const static QString KEYWORD_ELSEIF;
    const static QString KEYWORD_ENDIF;
    const static QString KEYWORD_LOOP;
    const static QString KEYWORD_ENDLOOP;
    const static QString KEYWORD_EXITWHEN;
    const static QString KEYWORD_GLOBALS;
    const static QString KEYWORD_ENDGLOBALS;
    const static QString KEYWORD_ARRAY;
    const static QString KEYWORD_CONSTANT;
    const static QString KEYWORD_TYPE;
    const static QString KEYWORD_EXTENDS;
    const static QString KEYWORD_NATIVE;
    const static QString KEYWORD_NULL;
    const static QString KEYWORD_NOT;
    const static QString KEYWORD_AND;
    const static QString KEYWORD_OR;
    const static QString KEYWORD_TRUE;
    const static QString KEYWORD_FALSE;
    // For a faster access we use this list with all keywords.
    const static QStringList KEYWRODS_ALL;

    // keep Warcraft III's default stuff in memory to improve the performance on highlighting
    const static QSet<QString> COMMONJ_TYPES_ALL;
    const static QSet<QString> COMMONJ_NATIVES_ALL;
    const static QSet<QString> COMMONJ_CONSTANTS_ALL;
    const static QSet<QString> BLIZZARDJ_CONSTANTS_ALL;
    const static QSet<QString> BLIZZARDJ_GLOBALS_ALL;
    const static QSet<QString> BLIZZARDJ_FUNCTIONS_ALL;
    const static QSet<QString> COMMONAI_NATIVES_ALL;
    const static QSet<QString> COMMONAI_CONSTANTS_ALL;
    const static QSet<QString> COMMONAI_GLOBALS_ALL;
    const static QSet<QString> COMMONAI_FUNCTIONS_ALL;

    const static QRegularExpression IDENTIFIER_REGEX;

    enum Type {
        FunctionKeyword,
        TakesKeyword,
        NothingKeyword,
        ReturnsKeyword,
        ReturnKeyword,
        LocalKeyword,
        SetKeyword,
        CallKeyword,
        IfKeyword,
        ThenKeyword,
        ElseifKeyword,
        EndifKeyword,
        LoopKeyword,
        EndloopKeyword,
        ExitwhenKeyword,
        EndfunctionKeyword,
        ConstantKeyword,
        TypeKeyword,
        ExtendsKeyword,
        NativeKeyword,
        GlobalsKeyword,
        EndglobalsKeyword,
        ArrayKeyword,
        NullKeyword,
        NotKeyword,
        AndKeyword,
        OrKeyword,
        TrueKeyword,
        FalseKeyword,
        Comment,
        Operator,
        AssignmentOperator,
        LeftBracket,
        RightBracket,
        LeftSquareBracket,
        RightSquareBracket,
        IntegerLiteral,
        RealLiteral,
        RawCodeLiteral,
        StringLiteral,
        EscapeLiteral,
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

    int getLength() const;
    bool highlight() const;

    bool isValidType() const;
    bool isValidIdentifier() const;
    bool isValidKeyword() const;

    static VJassToken::Type typeFromKeyword(const QString &keyword);

    int getValueLength() const;

    bool isCommonJType() const;
    bool isCommonJNative() const;
    bool isCommonJConstant() const;
    bool isBlizzardJConstant() const;
    bool isBlizzardJGlobal() const;
    bool isBlizzardJFunction() const;
    bool isCommonAINative() const;
    bool isCommonAIConstant() const;
    bool isCommonAIGlobal() const;
    bool isCommonAIFunction() const;

private:
    QString value;
    int line = 0;
    int column = 0;
    Type type;

    // cached stuff for faster access
    int valueLengthCached;

    enum CachedType {
        NONE,
        COMMONJ_TYPE,
        COMMONJ_NATIVE,
        COMMONJ_CONSTANT,
        BLIZZARDJ_CONSTANT,
        BLIZZARDJ_GLOBAL,
        BLIZZARDJ_FUNCTION,
        COMMONAI_NATIVE,
        COMMONAI_CONSTANT,
        COMMONAI_GLOBAL,
        COMMONAI_FUNCTION
    };

    CachedType cachedType;
};

#endif // VJASSTOKEN_H
