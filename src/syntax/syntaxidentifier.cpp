#include "syntaxidentifier.h"

namespace gams {
namespace studio {

SyntaxIdentifier::SyntaxIdentifier()
{
//    mSubStates << SyntaxState::
}

SyntaxBlock SyntaxIdentifier::find(SyntaxState entryState, const QString& line, int index)
{
    int start = index;
    while (isWhitechar(line, start))
        ++start;
    return SyntaxBlock();
}

} // namespace studio
} // namespace gams
