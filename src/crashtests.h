#pragma once
#include <QStringList>

class QMenu;

// Deliberate crashes and hangs for testing CrashHandler.
class CrashTests
{
public:
    CrashTests() = delete;

    static void addToMenu(QMenu* menu); // one action per test, note as tooltip/status tip
    static bool run(const QString& id); // false if id is unknown
    static QStringList ids();
};
