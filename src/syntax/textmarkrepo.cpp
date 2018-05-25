#include "textmarkrepo.h"

namespace gams {
namespace studio {

TextMarkRepo::TextMarkRepo(FileMetaRepo *fileRepo, QObject *parent)
    : QObject(parent), mFileRepo(fileRepo)
{

}

} // namespace studio
} // namespace gams
