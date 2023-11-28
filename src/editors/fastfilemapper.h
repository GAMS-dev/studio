#ifndef GAMS_STUDIO_FASTFILEMAPPER_H
#define GAMS_STUDIO_FASTFILEMAPPER_H

#include "abstracttextmapper.h"

namespace gams {
namespace studio {

class FastFileMapper : public AbstractTextMapper
{
    Q_OBJECT
public:
    explicit FastFileMapper(QObject *parent = nullptr);
    ~FastFileMapper() override;

    virtual AbstractTextMapper::Kind kind() const;
    virtual void startRun();
    virtual void endRun();

    virtual bool setVisibleTopLine(double region);
    virtual bool setVisibleTopLine(int lineNr);
    virtual int moveVisibleTopLine(int lineDelta);
    virtual int visibleTopLine() const;
    virtual void scrollToPosition();

    virtual int lineCount() const;
    virtual int knownLineNrs() const;

    virtual QString lines(int localLineNrFrom, int lineCount) const;
    virtual QString lines(int localLineNrFrom, int lineCount, QVector<LineFormat> &formats) const;
    virtual bool findText(QRegularExpression searchRegex, QTextDocument::FindFlags flags, bool &continueFind);

    virtual QString selectedText() const;
    virtual QString positionLine() const;

    virtual void setPosRelative(int localLineNr, int charNr, QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    virtual void setPosToAbsStart(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    virtual void setPosToAbsEnd(QTextCursor::MoveMode mode = QTextCursor::MoveAnchor);
    virtual void selectAll();
    virtual void clearSelection();
    virtual QPoint position(bool local = false) const;
    virtual QPoint anchor(bool local = false) const;
    virtual bool hasSelection() const;
    virtual int selectionSize() const;

    virtual bool atTail();
    virtual void updateSearchSelection();
    virtual void clearSearchSelection();

    virtual QPoint searchSelectionStart();
    virtual QPoint searchSelectionEnd();
    virtual void dumpPos() const;

public slots:
    virtual void reset();

protected:
    virtual bool updateMaxTop();

private:

};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FASTFILEMAPPER_H
