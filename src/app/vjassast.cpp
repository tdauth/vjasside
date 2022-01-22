#include <QtCore>

#include "vjassast.h"
#include "vjassparseerror.h"

VJassAst::VJassAst(int line, int column)
    : errors()
    , children()
    , codeCompletionSuggestions()
    , line(line)
    , column(column)
    , comments()
{
}

VJassAst::VJassAst(const VJassAst &other)
    : errors(other.getParseErrors())
  , children(other.getChildren())
  , codeCompletionSuggestions(other.getCodeCompletionSuggestions())
  , line(other.getLine())
  , column(other.getColumn())
  , comments(other.getComments())
{
}

VJassAst& VJassAst::operator=(const VJassAst &other) {
    this->errors = other.getParseErrors();
    this->children = other.getChildren();
    this->codeCompletionSuggestions = other.getCodeCompletionSuggestions();
    this->line = other.getLine();
    this->column = other.getColumn();
    this->comments = other.getComments();

    return *this;
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


const QList<VJassParseError>& VJassAst::getParseErrors() const {
    return errors;
}

QList<VJassParseError> VJassAst::getAllParseErrors() const {
    QStack<const VJassAst*> all;
    all.push_back(this);
    QList<VJassParseError> result;

    while (all.size() > 0) {
        const VJassAst *a = all.pop();

        for (QList<VJassAst*>::const_reference c : a->getChildren()) {
            all.push_back(c);
        }

        for (const VJassParseError &vjassParseError : a->getParseErrors()) {
            result.push_back(vjassParseError);
        }
    }

    return result;
}

const QList<VJassAst*>& VJassAst::getChildren() const {
    return children;
}

const QList<VJassAst*>& VJassAst::getCodeCompletionSuggestions() const {
    return codeCompletionSuggestions;
}

int VJassAst::getLine() const {
    return line;
}

int VJassAst::getColumn() const {
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

const QList<QString>& VJassAst::getComments() const {
    return comments;
}

QString VJassAst::toString() const {
    QString result = "";

    for (VJassAst *child : getChildren()) {
        result += child->toString();
    }

    return result;
}
