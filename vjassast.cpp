#include <QtCore>

#include "vjassast.h"
#include "vjassparseerror.h"

VJassAst::VJassAst(int line, int column) : line(line), column(column)
{
}

VJassAst::~VJassAst() {
    for (VJassAst *child : children) {
        delete child;
        child = nullptr;
    }

    for (VJassAst *codeCompletionSuggestion : codeCompletionSuggestions) {
        delete codeCompletionSuggestion;
        codeCompletionSuggestion = nullptr;
    }

    children.clear();
    codeCompletionSuggestions.clear();
}


QList<VJassParseError> VJassAst::getParseErrors() {
    return errors;
}

QList<VJassParseError> VJassAst::getAllParseErrors() {
    QList<VJassAst*> all;
    QStack<VJassAst*> children;
    children.push_back(this);

    while (children.size() > 0) {
        VJassAst *child = children.pop();
        all.push_back(child);

        for (VJassAst *c : child->getChildren()) {
            children.push_back(c);
        }
    }

    QList<VJassParseError> result;

    for (VJassAst *a : all) {
        for (VJassParseError vjassParseError : a->getParseErrors()) {
            result.push_back(vjassParseError);
        }
    }

    return result;
}

QList<VJassAst*> VJassAst::getChildren() {
    return children;
}

QList<VJassAst*> VJassAst::getCodeCompletionSuggestions() {
    return codeCompletionSuggestions;
}

int VJassAst::getLine() {
    return line;
}

int VJassAst::getColumn() {
    return column;
}

void VJassAst::addError(int line, int column, const QString &error) {
    this->errors.push_back(VJassParseError(line, column, error));
}

void VJassAst::addError(const VJassToken &token, const QString &error) {
    this->errors.push_back(VJassParseError(token.getLine(), token.getColumn(), error));
}

void VJassAst::addErrorAtEndOf(const VJassToken &token, const QString &error) {
    this->errors.push_back(VJassParseError(token.getLine(), token.getColumn() + token.getValue().length(), error));
}


void VJassAst::addChild(VJassAst *child) {
    this->children.push_back(child);
}

void VJassAst::addCodeCompletionSuggestion(VJassAst *codeCompletionSuggestion) {
    this->codeCompletionSuggestions.push_back(codeCompletionSuggestion);
}

void VJassAst::addComment(const QString &comment) {
    comments.push_back(comment);
}

const QList<QString>& VJassAst::getComments() {
    return comments;
}

QString VJassAst::toString() {
    QString result = "";

    for (VJassAst *child : getChildren()) {
        result += child->toString();
    }

    return result;
}
