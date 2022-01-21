#ifndef VJASSTOKEN_H
#define VJASSTOKEN_H

#include <QString>
#include <QStringList>

class VJassToken
{
public:
    const static QString KEYWORD_FUNCTION;
    const static QString KEYWORD_TAKES;
    const static QString KEYWORD_NOTHING;
    const static QString KEYWORD_RETURNS;
    const static QString KEYWORD_ENDFUNCTION;
    const static QString KEYWORD_GLOBALS;
    const static QString KEYWORD_CONSTANT;
    const static QStringList KEYWRODS_ALL;

    enum Type {
        FunctionKeyword,
        TakesKeyword,
        NothingKeyword,
        ReturnsKeyword,
        EndfunctionKeyword,
        Comment,
        Operator,
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

    static VJassToken::Type typeFromKeyword(const QString &keyword);

private:
    QString value;
    int line;
    int column;
    Type type;
};

#endif // VJASSTOKEN_H
