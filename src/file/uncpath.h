#ifndef UNCPATH_H
#define UNCPATH_H

#include <QObject>
#include <QSet>

namespace gams {
namespace studio {
namespace file {

class UncPath : public QObject
{
    Q_OBJECT
public:

    ///
    /// \brief Singleton constructor for UncPath
    /// \return
    ///
    static UncPath* unc();

    ///
    /// \brief Release UncPath singleton
    ///
    static void releaseUnc();

    ///
    /// \brief Destructor
    ///
    ~UncPath() override;

    ///
    /// \brief Translates a UNC path to a local path with a mapped drive letter (Windows only)
    /// \param uncPath the UNC path
    /// \param forceGenerate if no match is found, generate a mapping for a free drive letter
    /// \return The path translated to a mapped drive
    ///
    QString toMappedPath(const QString &uncPath, bool forceGenerate = false);

    ///
    /// \brief Generate a mapping for a free drive letter (Windows only)
    /// \param localDrive The drive letter to map to
    /// \param uncPath the UNC path. Credentials needs to be already unlocked.
    /// \return TRUE if the mapping suceeded
    ///
    bool mapNetworkDrive(const QString &localDrive, const QString &uncPath);

signals:
    // remark: Signal destroyed is used to set the static member mUnc to nullptr.

private:
    static UncPath *mUnc;
    QStringList mTempMapped;
    explicit UncPath(QObject *parent = nullptr);

private:
    bool unmapDrive(const QString &driveLetter, bool force = false);
};

} // namespace file
} // namespace studio
} // namespace gams

#endif // UNCPATH_H
