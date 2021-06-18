#include "syntaxsimulator.h"

SyntaxSimulator::SyntaxSimulator() : QObject()
{}

void SyntaxSimulator::clearBlockSyntax()
{
    mBlockSyntax.clear();
}

void SyntaxSimulator::addBlockSyntax(int pos, gams::studio::syntax::SyntaxKind syntax, int flavor)
{
    mBlockSyntax.insert(pos, QPair<int,int>(int(syntax), flavor));
}

void SyntaxSimulator::scanSyntax(QTextBlock block, QMap<int, QPair<int, int> > &blockSyntax)
{
    Q_UNUSED(block) // the blocks content is simulated
    blockSyntax = mBlockSyntax;
}
