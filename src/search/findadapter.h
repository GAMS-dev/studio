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

class QTextBrowser;

#ifdef QWEBENGINE
class QWebEngineView;
#endif

namespace gams {
namespace studio {

class CodeEdit;
class TextView;

namespace find {

enum FindOption {
    foNone       = 0x00,
    foFocusEdit  = 0x01,
    foFocusTerm  = 0x02,
    foBackwards  = 0x04,
    foExactMatch = 0x08,
    foCaseSense  = 0x10,
    foContinued  = 0x20,
    foSkipFind   = 0x40,    // At start of search when the selection is already set
};
typedef QFlags<FindOption> FindOptions;

///
/// \brief The FindAdapter class: Adapter classes to communicate with different widget types
///
/// Derivates of the FindAdapter can be created for supported widgets using the static method createAdapter
///
class FindAdapter : public QObject
{
    Q_OBJECT
public:
    ///
    /// \brief createAdapter static constructor of a FindAdapter matching to the given widget
    /// \param widget the widget the FindAdapter is created for
    /// \return A FindAdapter matching to the type of the widget or nullptr if the type is unsupported
    ///
    static FindAdapter *createAdapter(QWidget *widget);

    ///
    /// \brief widget Getter for the assigned widget
    ///
    virtual QWidget *widget() const;

    ///
    /// \brief setFocus Sets the focus to the assigned widget
    ///
    void setFocus();

    ///
    /// \brief canReplace Checks if replace is supported, i.e. the editor isn't read-only
    ///
    virtual bool canReplace() const;

    ///
    /// \brief hasSelectedFind Checks if the widget has a selection that results from a find action
    ///
    virtual bool hasSelectedFind() const = 0;

    ///
    /// \brief setFindTerm Sets the findTerm and triggers a find action
    /// \param rex The findTerm
    /// \param options Options for the find action
    ///
    virtual void setFindTerm(const QRegularExpression &rex, FindOptions options) = 0;

    ///
    /// \brief hasFindTerm Checks if the widget has a find term
    ///
    virtual bool hasFindTerm() = 0;

    ///
    /// \brief findText Triggers a find action
    /// \param rex The find term
    /// \param options Options for the find action
    /// \return TRUE if the find term has been found
    ///
    virtual bool findText(const QRegularExpression &rex, FindOptions options) = 0;

    ///
    /// \brief findText Triggers a find action
    /// \param text The find term
    /// \param options Options for the find action
    /// \return TRUE if the find term has been found
    ///
    virtual bool findText(const QString &text, FindOptions options);

    ///
    /// \brief findReplaceAll Replace all occurrencies in the content of the widget
    /// \param rex The find term
    /// \param options Options for the find action
    /// \param replacement The replacement text
    /// \return The count of replaced sections
    ///
    virtual int findReplaceAll(const QRegularExpression &rex, FindOptions options, const QString &replacement);

    ///
    /// \brief findReplace Replace the current find selection with the replacement
    /// \param replacement The replacement text
    /// \return TRUE if the replace has been successful
    ///
    virtual bool findReplace(const QString &replacement);

    ///
    /// \brief currentFindSelection Gets the current selection resulting from a find action. Other selections are ignored
    ///
    virtual QString currentFindSelection(bool &isCurrentWord) = 0;

    ///
    /// \brief invalidateSelection Performs a deselect in the widget if the selection is a result from a find action
    ///
    virtual void invalidateSelection() = 0;

    ///
    /// \brief RegExSupported determines if this adapter supports QRegularExpression
    /// \return TRUE by default
    ///
    virtual bool supportsRegEx() { return true; }

signals:
    ///
    /// \brief allowReplaceChanged Signals when the state of \a hasSelection has changed
    ///
    void allowReplaceChanged();

    ///
    /// \brief endFind Signals when the find should be closed
    ///
    void endFind();

    ///
    /// \brief findNext Signals find next/previous for adapters whose widget doesn't implement it
    /// \param backwards
    ///
    void findNext(bool backwards);

protected slots:
    void widgetDestroyed();

protected:
    // friend class EditFindAdapter;
    explicit FindAdapter(QWidget *widget = nullptr);
    QTextDocument::FindFlags findFlags(FindOptions options);
};


// ---------------------


class EditFindAdapter : public FindAdapter
{
    Q_OBJECT
public:
    ~EditFindAdapter() override;
    QWidget *widget() const override;
    bool canReplace() const override;
    bool hasSelectedFind() const override;
    void setFindTerm(const QRegularExpression &rex, FindOptions options) override;
    bool hasFindTerm() override;
    bool findText(const QRegularExpression &rex, FindOptions options) override;
    int findReplaceAll(const QRegularExpression &rex, FindOptions options, const QString &replacement) override;
    bool findReplace(const QString &replacement) override;
    QString currentFindSelection(bool &isCurrentWord) override;
    void invalidateSelection() override;

protected:
    friend class FindAdapter;
    EditFindAdapter(gams::studio::CodeEdit *edit = nullptr);

private:
    CodeEdit *mEdit;
};


// ------------------


class ViewFindAdapter : public FindAdapter
{
    Q_OBJECT
public:
    ~ViewFindAdapter() override;
    QWidget *widget() const override;
    bool hasSelectedFind() const override;
    void setFindTerm(const QRegularExpression &rex, FindOptions options) override;
    bool hasFindTerm() override;
    bool findText(const QRegularExpression &rex, FindOptions options) override;
    QString currentFindSelection(bool &isCurrentWord) override;
    void invalidateSelection() override;

protected:
    friend class FindAdapter;
    ViewFindAdapter(gams::studio::TextView *view = nullptr);

private:
    TextView *mView;
};


// ------------------


class ChangelogFindAdapter : public FindAdapter
{
    Q_OBJECT
public:
    ~ChangelogFindAdapter() override;
    QWidget *widget() const override;
    bool hasSelectedFind() const override;
    void setFindTerm(const QRegularExpression &rex, FindOptions options) override;
    bool hasFindTerm() override;
    bool findText(const QRegularExpression &rex, FindOptions options) override;
    QString currentFindSelection(bool &isCurrentWord) override;
    void invalidateSelection() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    friend class FindAdapter;
    ChangelogFindAdapter(QTextBrowser *view = nullptr);
    void calcExtraSelections();

private:
    QTextBrowser *mView;
    QRegularExpression *mRex = nullptr;
    FindOptions mOptions = foNone;
    bool mTakeSelection = false;
    QString mSelection;
};


// ------------------


#ifdef QWEBENGINE
class WebViewFindAdapter : public FindAdapter
{
    Q_OBJECT
public:
    ~WebViewFindAdapter() override;
    QWidget *widget() const override;
    bool hasSelectedFind() const override;
    void setFindTerm(const QRegularExpression &rex, FindOptions options) override;
    bool hasFindTerm() override;
    bool findText(const QRegularExpression &rex, FindOptions options) override;
    bool findText(const QString &text, FindOptions options) override;
    QString currentFindSelection(bool &isCurrentWord) override;
    void invalidateSelection() override;
    bool supportsRegEx() override { return false; }
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    friend class FindAdapter;
    WebViewFindAdapter(QWebEngineView *view = nullptr);

private:
    QWebEngineView *mView;
};
#endif

} // namespace find
} // namespace studio
} // namespace gams

#endif // FINDADAPTER_H
