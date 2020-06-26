#include "navigationhistorylocator.h"
#include "navigationhistory.h"

namespace gams {
namespace studio {

NavigationHistory* NavigationHistoryLocator::mNh = nullptr;

NavigationHistory *NavigationHistoryLocator::navigationHistory()
{
    return mNh;
}

void NavigationHistoryLocator::provide(NavigationHistory *navHistory)
{
    mNh = navHistory;
}


}
}
