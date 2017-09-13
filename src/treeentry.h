#ifndef TREEENTRY_H
#define TREEENTRY_H

#include <QtCore>

namespace gams {
namespace ide {

class TreeEntry : public QObject
{
    Q_OBJECT
public:
    TreeEntry(TreeEntry* par, QString name, QString identString, bool isGist = false);
    ~TreeEntry();

    bool isGist() const;
    const QString& name() const;
    void setName(const QString& name);
    const QString& identString() const;
    bool matches(const QString& name, bool isGist) const;
    TreeEntry* child(int index) const;
    TreeEntry* parentEntry() const;
    int peekIndex(QString name, bool skipLast = false);

private:
    QString mName;
    QString mIdentString;
    bool mIsGist;
};

} // namespace ide
} // namespace gams

#endif // TREEENTRY_H
