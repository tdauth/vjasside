#ifndef VJASSAST_H
#define VJASSAST_H

#include <QList>

#include "vjassparseerror.h"
#include "vjasstoken.h"

class VJassAst
{
public:
    VJassAst(int line, int column);
    virtual ~VJassAst();

    QList<VJassParseError> getParseErrors();
    QList<VJassParseError> getAllParseErrors();
    QList<VJassAst*> getChildren();
    QList<VJassAst*> getCodeCompletionSuggestions();
    int getLine();
    int getColumn();

    void addError(int line, int column, const QString &error);
    void addError(const VJassToken &token, const QString &error);
    void addErrorAtEndOf(const VJassToken &token, const QString &error);
    void addChild(VJassAst *child);
    void addCodeCompletionSuggestion(VJassAst *codeCompletionSuggestion);

    void addComment(const QString &comment);
    const QList<QString>& getComments();

    virtual QString toString();

private:
    QList<VJassParseError> errors;
    QList<VJassAst*> children;
    QList<VJassAst*> codeCompletionSuggestions;
    int line;
    int column;
    QList<QString> comments;
};

#endif // VJASSAST_H
