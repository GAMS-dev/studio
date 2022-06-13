#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H

#include <QStringList>


namespace gams {
namespace studio {
namespace gdxviewer {

struct ValueFilterState {
    bool active = false;
    double min;
    double max;
    bool exclude;
    bool showUndef;
    bool showNA;
    bool showPInf;
    bool showMInf;
    bool showEps;
    bool showAcronym;
};

class GdxSymbolViewState
{
public:
    GdxSymbolViewState();

    bool sqTrailingZeroes() const;
    void setSqTrailingZeroes(bool squeezeTrailingZeroes);

    int dim() const;
    void setDim(int dim);

    int type() const;
    void setType(int type);

    bool tableViewActive() const;
    void setTableViewActive(bool tableViewActive);

    QVector<QStringList> uncheckedLabels() const;

    void setUncheckedLabels(const QVector<QStringList> &uncheckedLabels);

    int numericalPrecision() const;
    void setNumericalPrecision(int numericalPrecision);

    bool restoreSqZeroes() const;
    void setRestoreSqZeroes(bool restoreSqZeroes);

    int valFormatIndex() const;
    void setValFormatIndex(int valFormatIndex);

    bool sqDefaults() const;
    void setSqDefaults(bool sqDefaults);

    QByteArray listViewHeaderState() const;
    void setListViewHeaderState(const QByteArray &listViewHeaderState);

    QByteArray tableViewFilterHeaderState() const;
    void setTableViewFilterHeaderState(const QByteArray &tableViewFilterHeaderState);

    bool tableViewLoaded() const;
    void setTableViewLoaded(bool tableViewLoaded);

    int tvColDim() const;
    void setTvColDim(int tvColDim);

    QVector<int> tvDimOrder() const;
    void setTvDimOrder(const QVector<int> &tvDimOrder);

    QVector<ValueFilterState> valueFilterState() const;
    void setValueFilterState(const QVector<ValueFilterState> &valueFilterState);

    QVector<bool> getShowAttributes() const;
    void setShowAttributes(const QVector<bool> &value);

    QVector<int> getTableViewColumnWidths() const;
    void setTableViewColumnWidths(const QVector<int> &tableViewColumnWidths);

private:
    bool mSqDefaults;
    bool mSqTrailingZeroes;
    bool mRestoreSqZeroes;
    bool mTableViewActive;
    bool mTableViewLoaded = false;
    int mNumericalPrecision;
    int mValFormatIndex;
    int mDim;
    int mType;

    QVector<bool> showAttributes;

    // table view state
    int mTvColDim;
    QVector<int> mTvDimOrder;

    // column filters
    QVector<QStringList> mUncheckedLabels;

    // value filters
    QVector<ValueFilterState> mValueFilterState;

    QByteArray mListViewHeaderState;
    QByteArray mTableViewFilterHeaderState;
    QVector<int> mTableViewColumnWidths;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
