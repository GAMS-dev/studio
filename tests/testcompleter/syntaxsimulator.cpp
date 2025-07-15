/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
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
