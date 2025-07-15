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
#ifndef SYNTAXSIMULATOR_H
#define SYNTAXSIMULATOR_H

#include <QObject>
#include <QTextBlock>
#include "syntax/syntaxformats.h"

class SyntaxSimulator : public QObject
{
    Q_OBJECT
    QMap<int, QPair<int,int>> mBlockSyntax;

public:
    explicit SyntaxSimulator();
    void clearBlockSyntax();
    void addBlockSyntax(int pos, gams::studio::syntax::SyntaxKind syntax, int flavor);

public slots:
    void scanSyntax(QTextBlock block, QMap<int, QPair<int,int>> &blockSyntax);

};

#endif // SYNTAXSIMULATOR_H
