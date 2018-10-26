#ifndef PAGINGTEXTVIEW_H
#define PAGINGTEXTVIEW_H

#include <QTableView>

#include "common.h"
#include "pagingtextmodel.h"

namespace gams {
namespace studio {

class PagingTextView: public QTableView
{
    Q_OBJECT
public:
    explicit  PagingTextView(QWidget *parent);

    void loadFile(QString fileName);

    FileId fileId() const;
    void setFileId(const FileId &fileId);
    NodeId groupId() const;
    virtual void setGroupId(const NodeId &groupId);

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    void zoomInF(float range);
    void scrollTo(const QModelIndex &index, ScrollHint hint) override;

public slots:
    void reorganize();

protected:
    bool event(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;
    void showEvent(QShowEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void calculateWidth();
    void adjustWidth();

private:
    struct WidthData { int top=0; int bottom=0; int width=0; };
    PagingTextModel mModel;
    FileId mFileId;
    NodeId mGroupId;
    WidthData mLastWidthData;

};

} // namespace studio
} // namespace gams

#endif // PAGINGTEXTVIEW_H
