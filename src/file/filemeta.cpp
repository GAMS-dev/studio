#include "filemeta.h"

namespace gams {
namespace studio {

FileMeta::FileMeta(QString location) : mLocation(location)
{

}

QString FileMeta::location() const
{
    return mLocation;
}

} // namespace studio
} // namespace gams
