#ifndef GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
#define GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H

#include <QStringList>


namespace gams {
namespace studio {
namespace gdxviewer {

class GdxSymbolViewState
{
public:
    GdxSymbolViewState();

    bool squeezeTrailingZeroes() const;
    void setSqueezeTrailingZeroes(bool squeezeTrailingZeroes);

    int dim() const;
    void setDim(int dim);

    int type() const;
    void setType(int type);

    bool tableView() const;
    void setTableView(bool tableView);

    QVector<QStringList> uncheckedLabels() const;

    void setUncheckedLabels(const QVector<QStringList> &uncheckedLabels);

    QVector<double> currentMin() const;
    void setCurrentMin(const QVector<double> &currentMin);

    QVector<double> currentMax() const;
    void setCurrentMax(const QVector<double> &currentMax);

    QVector<bool> valFilterActive() const;
    void setValFilterActive(const QVector<bool> &valFilterActive);

    QVector<bool> exclude() const;
    void setExclude(const QVector<bool> &exclude);

    QVector<bool> showUndef() const;
    void setShowUndef(const QVector<bool> &showUndef);

    QVector<bool> showNA() const;
    void setShowNA(const QVector<bool> &showNA);

    QVector<bool> showPInf() const;
    void setShowPInf(const QVector<bool> &showPInf);

    QVector<bool> showMInf() const;
    void setShowMInf(const QVector<bool> &showMInf);

    QVector<bool> showEps() const;
    void setShowEps(const QVector<bool> &showEps);

    QVector<bool> showAcronym() const;
    void setShowAcronym(const QVector<bool> &showAcronym);

    int numericalPrecision() const;
    void setNumericalPrecision(int numericalPrecision);

    bool restoreSqZeros() const;
    void setRestoreSqZeros(bool restoreSqZeros);

    int valFormatIndex() const;
    void setValFormatIndex(int valFormatIndex);

    bool sqDefaults() const;
    void setSqDefaults(bool sqDefaults);

private:
    bool mSqDefaults;
    bool mSqueezeTrailingZeroes;
    bool mRestoreSqZeros;
    bool mTableView;
    int mNumericalPrecision;
    int mValFormatIndex;
    int mDim;
    int mType;

    // column filters
    QVector<QStringList> mUncheckedLabels;

    // value filters
    QVector<bool> mValFilterActive;
    QVector<double> mCurrentMin;
    QVector<double> mCurrentMax;
    QVector<bool> mExclude;
    QVector<bool> mShowUndef;
    QVector<bool> mShowNA;
    QVector<bool> mShowPInf;
    QVector<bool> mShowMInf;
    QVector<bool> mShowEps;
    QVector<bool> mShowAcronym;
};

} // namespace gdxviewer
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_GDXVIEWER_GDXSYMBOLVIEWSTATE_H
