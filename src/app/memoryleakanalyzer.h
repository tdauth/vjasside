#ifndef MEMORYLEAKANALYZER_H
#define MEMORYLEAKANALYZER_H

#include "vjassast.h"
#include "vjassglobal.h"

/**
 * @brief Overapproximates possible memory leaks by looking for memory release calls with matching identifiers.
 */
class MemoryLeakAnalyzer
{
public:
    MemoryLeakAnalyzer(VJassAst *ast);

    const QList<VJassGlobal*>& getGlobals() const;

private:
    QList<VJassGlobal*> globals;
};

#endif // MEMORYLEAKANALYZER_H
