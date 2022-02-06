#include "memoryleakanalyzer.h"
#include "vjassast.h"
#include "vjassglobal.h"
#include "vjassexpression.h"

const QMap<QString, QString> MemoryLeakAnalyzer::LEAKING_TYPES = {
    { "location", "RemoveLocation" },
    { "rect", "RemoveRect" },
    { "unit", "RemoveUnit" },
    { "trigger", "DestroyTrigger" },
    { "effect", "DestroyEffect" },
    { "group", "DestroyGroup" },
    { "weathereffect", "RemoveWeatherEffect" },
    { "effect", "DestroyEffect" },
    { "lightning", "DestroyLightning" },
    { "image", "DestroyImage" },
    { "ubersplat", "DestroyUbersplat" },
    { "fogmodifier", "DestroyFogModifier" },
    { "item", "RemoveItem" },
    { "destructable", "RemoveDestructable" }
};

MemoryLeakAnalyzer::MemoryLeakAnalyzer(VJassAst *ast)
{
    QList<VJassAst*> globals = ast->getAllMatching([](VJassAst *ast) {
        return typeid(*ast) == typeid(VJassGlobal) && LEAKING_TYPES.contains(dynamic_cast<VJassGlobal*>(ast)->getType());
    });

    QList<VJassAst*> calls = ast->getAllMatching([](VJassAst *ast) {
        return typeid(*ast) == typeid(VJassExpression)
            && dynamic_cast<VJassExpression*>(ast)->getType() == VJassExpression::FunctionCall
            && LEAKING_TYPES.values().contains(dynamic_cast<VJassExpression*>(ast)->getValue());
    });


    for (VJassAst *g : globals) {
        VJassGlobal *global = dynamic_cast<VJassGlobal*>(g);

        QList<VJassAst*>::iterator iterator = std::find_if(calls.begin(), calls.end(), [global](VJassAst *a) {
            return a->getChildren().size() > 0
                && a->getChildren().at(0) != nullptr
                && typeid(VJassExpression) == typeid(*a->getChildren().at(0))
                && dynamic_cast<VJassExpression*>(a->getChildren().at(0))->getValue() == global->getName();
        });

        if (iterator == calls.end()) {
            globals.push_back(global);
        }
    }
}

const QList<VJassGlobal*>& MemoryLeakAnalyzer::getGlobals() const {
    return globals;
}
