#ifndef NAVIGATIONHISTORYLOCATOR_H
#define NAVIGATIONHISTORYLOCATOR_H

namespace gams {
namespace studio {

class NavigationHistory;
class NavigationHistoryLocator
{

public:
    static NavigationHistory* navigationHistory();
    static void provide(NavigationHistory* navHistory);

private:
    static NavigationHistory* mNh;

};

}
}
#endif // NAVIGATIONHISTORYLOCATOR_H
