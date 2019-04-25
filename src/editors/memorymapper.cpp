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

void MemoryMapper::closeAndReset(bool initAnchor)
{
    if (mChunks.size()) {
        mRecent << Recent(mChunks.size()-1);
    }
    // prepare new section
}

bool MemoryMapper::setMappingSizes(int bufferedLines, int chunkSizeInBytes, int chunkOverlap)
{
    return AbstractTextMapper::setMappingSizes(bufferedLines, chunkSizeInBytes, chunkOverlap);
}

void MemoryMapper::startRun()
{
    closeAndReset(false);
}

AbstractTextMapper::Chunk *MemoryMapper::getChunk(int chunkNr) const
{
    // clarify how to manage chunk-cache and internal chunks
    // 1. recentChunks = the count of chunks declared as recent
    // 2. first active chunk = recentChunk + 1
    // 3. pre-allocate full chunks
    // 4. define max-size in chunk-count
    // 5. when last chunk is full:
    //    a. mark last line of first active chunk with "..."
    //    b. append new data at last chunk
    //    c. reduce second active chunk
    return nullptr;
}

} // namespace studio
} // namespace gams
