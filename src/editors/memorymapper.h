#ifndef MEMORYMAPPER_H
#define MEMORYMAPPER_H

#include "abstracttextmapper.h"

namespace gams {
namespace studio {

class MemoryMapper : public AbstractTextMapper
{
    Q_OBJECT
private:
    struct Recent {
        Recent(int idx = -1, QString text = QString()) : index(idx), foldText(text), folded(true) {}
        int index;
        QString foldText;
        bool folded;
    };

public:
    explicit MemoryMapper(QObject *parent = nullptr);
    qint64 size() const override;
    bool setMappingSizes(int bufferedLines, int chunkSizeInBytes, int chunkOverlap) override;
    void startRun() override;

signals:

public slots:

protected:
    Chunk *getChunk(int chunkNr) const override;

private:
    void closeAndReset(bool initAnchor);


private:
    QVector<Chunk*> mChunks;
    QVector<Recent> mRecent;

};

} // namespace studio
} // namespace gams

#endif // MEMORYMAPPER_H
