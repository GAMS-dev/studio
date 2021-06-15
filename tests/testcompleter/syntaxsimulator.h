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
