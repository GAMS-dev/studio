#ifndef GAMS_STUDIO_FILTERLINEEDIT_H
#define GAMS_STUDIO_FILTERLINEEDIT_H

#include <QLineEdit>

namespace gams {
namespace studio {

class FilterLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit FilterLineEdit(QWidget *parent = nullptr);
    explicit FilterLineEdit(const QString &contents, QWidget *parent = nullptr);
    const QRegExp &regExp() const;

signals:
    void regExpChanged(const QRegExp &regExp);

public slots:
    void setExactMatch(bool exact);

private:
    void init();
    void updateRegExp();

private:
    QToolButton *mExactButton = nullptr;
    QToolButton *mClearButton = nullptr;
    bool mExact = false;
    QRegExp mRegExp;
};

} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_FILTERLINEEDIT_H
