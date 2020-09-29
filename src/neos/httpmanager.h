#ifndef GAMS_STUDIO_NEOS_HTTPMANAGER_H
#define GAMS_STUDIO_NEOS_HTTPMANAGER_H

#include <QtNetwork>

namespace gams {
namespace studio {
namespace neos {

class HttpManager: public QObject
{
    Q_OBJECT
public:
    HttpManager(QObject *parent = nullptr);
    void setUrl(const QString &url);
    void setIgnoreSslErrors();

signals:
    void received(QString name, QVariant data);
    void sslErrors(const QStringList &errorList);

public slots:
    void submitCall(const QString &method, const QVariantList &params = QVariantList());

private slots:
    void prepareReply(QNetworkReply *reply);
    void convertSslErrors(QNetworkReply *reply, const QList<QSslError> &errorList);

private:
    QNetworkRequest mRawRequest;
    QNetworkAccessManager mManager;
    bool mIgnoreSslErrors = false;
};

} // namespace neos
} // namespace studio
} // namespace gams

#endif // GAMS_STUDIO_NEOS_HTTPMANAGER_H
