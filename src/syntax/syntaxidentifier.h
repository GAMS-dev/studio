#ifndef SYNTAXIDENTIFIER_H
#define SYNTAXIDENTIFIER_H

#include "syntaxformats.h"

namespace gams {
namespace studio {

class SyntaxIdentifier : public SyntaxAbstract
{
public:
    SyntaxIdentifier();
    inline SyntaxState state() override { return SyntaxState::Identifier; }
    SyntaxBlock find(SyntaxState entryState, const QString &line, int index) override;
};

} // namespace studio
} // namespace gams

#endif // SYNTAXIDENTIFIER_H
