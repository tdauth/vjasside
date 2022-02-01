#include "memoryleakanalyzer.h"
#include "vjassast.h"
#include "vjassglobal.h"
#include "vjassexpression.h"

MemoryLeakAnalyzer::MemoryLeakAnalyzer(VJassAst *ast)
{
    QList<VJassAst*> locationGlobals = ast->getAllMatching([](VJassAst *ast) {
        return typeid(*ast) == typeid(VJassGlobal) && dynamic_cast<VJassGlobal*>(ast)->getType() == "location";
    });

    QList<VJassAst*> removeLocationCalls = ast->getAllMatching([](VJassAst *ast) {
        return typeid(*ast) == typeid(VJassExpression)
            && dynamic_cast<VJassExpression*>(ast)->getType() == VJassExpression::FunctionCall
            && dynamic_cast<VJassExpression*>(ast)->getValue() == "RemoveLocation";
    });

    for (VJassAst *l : locationGlobals) {
        VJassGlobal *locationGlobal = dynamic_cast<VJassGlobal*>(l);

        QList<VJassAst*>::iterator iterator = std::find_if(removeLocationCalls.begin(), removeLocationCalls.end(), [locationGlobal](VJassAst *a) {
            return a->getChildren().size() > 0
                && a->getChildren().at(0) != nullptr
                && typeid(VJassExpression) == typeid(*a->getChildren().at(0))
                && dynamic_cast<VJassExpression*>(a->getChildren().at(0))->getValue() == locationGlobal->getName();
        });

        if (iterator == removeLocationCalls.end()) {
            globals.push_back(locationGlobal);
        }
    }
}

const QList<VJassGlobal*>& MemoryLeakAnalyzer::getGlobals() const {
    return globals;
}
