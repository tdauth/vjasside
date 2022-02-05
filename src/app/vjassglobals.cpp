#include "vjassglobals.h"

VJassGlobals::VJassGlobals(int line, int column) : VJassAst(line, column)
{
}

VJassGlobals::~VJassGlobals() {

}

QString VJassGlobals::toString() const {
    QString result = "globals\n";

    for (VJassAst *child : getChildren()) {
        result += child->toString();
    }

    if (getChildren().size() > 0) {
        result += "\n";
    }

    result += "endglobals";

    return result;
}
