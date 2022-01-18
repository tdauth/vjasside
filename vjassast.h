#ifndef VJASSAST_H
#define VJASSAST_H

#include <QList>

#include "vjassparseerror.h"

class VJassAst
{
public:
    VJassAst(int line, int column);
    virtual ~VJassAst();

    QList<VJassParseError> getParseErrors();
    QList<VJassParseError> getAllParseErrors();
    QList<VJassAst*> getChildren();
    int getLine();
    int getColumn();

    void addError(int line, int column, const QString &error);
    void addChild(VJassAst *child);

    void addComment(const QString &comment);
    const QList<QString>& getComments();

private:
    QList<VJassParseError> errors;
    QList<VJassAst*> children;
    int line;
    int column;
    QList<QString> comments;
};

#endif // VJASSAST_H
