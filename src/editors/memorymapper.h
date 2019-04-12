#ifndef MEMORYMAPPER_H
#define MEMORYMAPPER_H

#include "abstracttextmapper.h"

namespace gams {
namespace studio {

class MemoryMapper : public AbstractTextMapper
{
    Q_OBJECT
public:
    explicit MemoryMapper(QObject *parent = nullptr);
    qint64 size() const override;

signals:

public slots:

protected:
    Chunk *getChunk(int chunkNr) const override;
    Chunk *loadChunk(int chunkNr) const override;

};

} // namespace studio
} // namespace gams

#endif // MEMORYMAPPER_H
