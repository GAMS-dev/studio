/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2026 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2026 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FINDADAPTER_H
#define FINDADAPTER_H

#include <QWidget>
#include <QTextDocument>

namespace gams {
namespace studio {

class CodeEdit;
class TextView;

namespace find {

enum FindOption {
    foFocusEdit  = 0x01,
    foFocusTerm  = 0x02,
    foBackwards  = 0x04,
    foExactMatch = 0x08,
    foCaseSense  = 0x16,
    foContinued  = 0x32,
};
typedef QFlags<FindOption> FindOptions;


class FindAdapter : public QObject
{
    Q_OBJECT
public:
    static FindAdapter *createAdapter(QWidget *widget);
    virtual QWidget *widget() const;
    void setFocus();
    virtual bool canReplace() const;
    virtual bool hasSelection() const = 0;
    virtual void setFindTerm(const QRegularExpression &rex, FindOptions options) = 0;
    virtual bool findText(const QRegularExpression &rex, FindOptions options) = 0;
    virtual int findReplaceAll(const QRegularExpression &rex, FindOptions options, const QString &replacement);
    virtual bool findReplace(const QRegularExpression &rex, FindOptions options, const QString &replacement);
    virtual QString currentFindSelection() const = 0;
    virtual void invalidateSelection() = 0;

signals:
    void allowReplaceChanged(QWidget *edit);
    void endFind();

protected slots:
    void widgetDestroyed();

protected:
    friend class EditFindAdapter;
    explicit FindAdapter(QWidget *widget = nullptr);
    QTextDocument::FindFlags findFlags(FindOptions options);
};


// ---------------------


class EditFindAdapter : public FindAdapter
{
    Q_OBJECT
public:
    EditFindAdapter(gams::studio::CodeEdit *edit = nullptr);
    ~EditFindAdapter() override;
    QWidget *widget() const override;
    bool canReplace() const override;
    bool hasSelection() const override;
    void setFindTerm(const QRegularExpression &rex, FindOptions options) override;
    bool findText(const QRegularExpression &rex, FindOptions options) override;
    int findReplaceAll(const QRegularExpression &rex, FindOptions options, const QString &replacement) override;
    bool findReplace(const QRegularExpression &rex, FindOptions options, const QString &replacement) override;
    QString currentFindSelection() const override;
    void invalidateSelection() override;

private:
    CodeEdit *mEdit;
};


// ------------------


class ViewFindAdapter : public FindAdapter
{
    Q_OBJECT
public:
    ViewFindAdapter(gams::studio::TextView *view = nullptr);
    ~ViewFindAdapter() override;
    QWidget *widget() const override;
    bool hasSelection() const override;
    void setFindTerm(const QRegularExpression &rex, FindOptions options) override;
    bool findText(const QRegularExpression &rex, FindOptions options) override;
    QString currentFindSelection() const override;
    void invalidateSelection() override;

private:
    TextView *mView;
};


} // namespace find
} // namespace studio
} // namespace gams

#endif // FINDADAPTER_H
