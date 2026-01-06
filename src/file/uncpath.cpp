#include "uncpath.h"
#include <QDebug>

#ifdef _WIN64

#include <windows.h>

namespace gams {
namespace studio {
namespace file {

UncPath *UncPath::mUnc = nullptr;

UncPath::UncPath(QObject *parent)
    : QObject{parent}
{
    connect(this, &UncPath::destroyed, this, []() {
        UncPath::mUnc = nullptr;
    });
}

UncPath *UncPath::unc()
{
    if (!mUnc)
        mUnc = new UncPath();
    return mUnc;
}

void UncPath::releaseUnc()
{
    delete mUnc;
}

UncPath::~UncPath()
{
    while (!mTempMapped.isEmpty()) {
        QString drive = *mTempMapped.begin();
        unmapDrive(drive);
        mTempMapped.removeAll(drive);
    }
}

QString UncPath::toMappedPath(const QString &uncPath, bool forceGenerate)
{
    if (uncPath.isEmpty())
        return QString();

    wchar_t lastFreeDrive = L' ';
    for (wchar_t drive = L'A'; drive <= L'Z'; ++drive) {
        QString driveLetter = QString("%1:").arg(QChar(drive));
        WCHAR buffer[MAX_PATH] = {0};
        DWORD size = MAX_PATH;

        DWORD res = WNetGetConnectionW(
            (LPCWSTR)driveLetter.utf16(),
            buffer,
            &size
            );

        if (res == NO_ERROR) {
            QString mappedUNC = QString::fromWCharArray(buffer);
            if (uncPath.startsWith(mappedUNC, Qt::CaseInsensitive)) {
                QString relPath = uncPath.mid(mappedUNC.length(), uncPath.length());
                return driveLetter + relPath;
            }
        } else {
            lastFreeDrive = drive;
        }
    }
    if (forceGenerate && lastFreeDrive != L' ') {
        // Generate temporary mapping for the parent directory of this UNC path
        QString tempUncPath = uncPath.left(uncPath.lastIndexOf('\\', -2));
        if (tempUncPath.count('\\') < 4)
            tempUncPath = uncPath;
        QChar newDrive = QChar(lastFreeDrive);
        if (mapNetworkDrive(newDrive + ':', tempUncPath)) {
            mTempMapped << (newDrive + ':');
            return toMappedPath(uncPath, false);
        }
    }

    return QString();
}

bool UncPath::mapNetworkDrive(const QString &localDrive, const QString &uncPath)
{
    NETRESOURCE nr;
    memset(&nr, 0, sizeof(nr));

    std::wstring drive = localDrive.toStdWString();
    std::wstring remote = uncPath.toStdWString();

    nr.dwType = RESOURCETYPE_DISK;
    nr.lpLocalName = const_cast<LPWSTR>(drive.c_str());
    nr.lpRemoteName = const_cast<LPWSTR>(remote.c_str());
    nr.lpProvider = NULL;

    // Use WNetAddConnection2 ignoring credentials (must be already connected)
    DWORD result = WNetAddConnection2(&nr, NULL, NULL, CONNECT_TEMPORARY);

    if (result != NO_ERROR) {
        qDebug() << "Error mapping drive. Error code:" << result;
        return false;
    }
    qDebug() << "Drive " << localDrive << " mapped to " << uncPath;
    return true;
}

bool UncPath::unmapDrive(const QString &driveLetter, bool force)
{
    if (driveLetter.isEmpty()) return false;

    DWORD result = WNetCancelConnection2(driveLetter.toStdWString().c_str(), 0, force);
    if (result != NO_ERROR) {
        qDebug() << "Failed to unmap" << driveLetter << "Error code:" << result;
        return false;
    }
    qDebug() << "Successfully unmapped" << driveLetter;
    return true;
}

#endif

} // namespace file
} // namespace studio
} // namespace gams
