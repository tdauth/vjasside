#ifndef VJASSAST_H
#define VJASSAST_H

#include <functional>

#include <QList>
#include <QString>

#include "vjassparseerror.h"
#include "vjasstoken.h"

class VJassAst
{
public:
    VJassAst(int line, int column);
    VJassAst(const VJassAst &other);
    VJassAst& operator=(const VJassAst &other);
    virtual ~VJassAst();

    const QList<VJassParseError>& getParseErrors() const;
    QList<VJassParseError> getAllParseErrors() const;
    const QList<VJassAst*>& getChildren() const;
    const QList<VJassAst*>& getCodeCompletionSuggestions() const;
    int getLine() const;
    int getColumn() const;

    void addError(int line, int column, int length, const QString &error);
    void addError(const VJassToken &token, const QString &error);
    void addErrorAtEndOf(const VJassToken &token, const QString &error);
    void addChild(VJassAst *child);
    void addCodeCompletionSuggestion(VJassAst *codeCompletionSuggestion);

    void addComment(const QString &comment);
    const QList<QString>& getComments() const;

    virtual QString toString() const;

    QList<VJassAst*> getAllMatching(std::function<bool(VJassAst*)> &&f);

    static void sortByPosition(QList<VJassAst*> &list);

private:
    QList<VJassParseError> errors;
    QList<VJassAst*> children;
    QList<VJassAst*> codeCompletionSuggestions;
    int line = 0;
    int column = 0;
    QList<QString> comments;
};

#endif // VJASSAST_H
