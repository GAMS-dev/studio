#ifndef GAMS_STUDIO_ENGINE_AUTHMANAGER_H
#define GAMS_STUDIO_ENGINE_AUTHMANAGER_H

#include <QObject>
#include <QTimer>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace gams {
namespace studio {
namespace engine {

class EngineManager;

class AuthManager: public QObject
{
    Q_OBJECT
public:
    AuthManager(EngineManager *engineManager, QObject *parent);
    ~AuthManager() override;
    void listProvider(const QString &name);

signals:
    void authorizeSignal(QNetworkReply *reply);
    void authorizeTimeout(QNetworkReply *reply);
    void reProviderList(const QList<QHash<QString, QVariant> > &allProvider);

public slots:
    void authorize(QUrl authUrl, const QString &clientId);

private slots:
    void reListProvider(const QList<QHash<QString, QVariant> > &allProvider);

private:
    QNetworkAccessManager *mNetworkManager;
    EngineManager *mEngineManager;
    QString mExplicitProvider;
    QTimer mTimeoutTimer;
};

} // namespace engine
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_ENGINE_AUTHMANAGER_H
