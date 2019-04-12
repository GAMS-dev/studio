#include "memorymapper.h"

namespace gams {
namespace studio {

MemoryMapper::MemoryMapper(QObject *parent) : AbstractTextMapper (parent)
{

}

qint64 MemoryMapper::size() const
{
    return 0;
}

AbstractTextMapper::Chunk *MemoryMapper::getChunk(int chunkNr) const
{
    return nullptr;
}

AbstractTextMapper::Chunk *MemoryMapper::loadChunk(int chunkNr) const
{
    return nullptr;
}

} // namespace studio
} // namespace gams
