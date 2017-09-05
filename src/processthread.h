#ifndef PROCESSTHREAD_H
#define PROCESSTHREAD_H

#include <QThread>

class ProcessThread : public QThread
{
    Q_OBJECT
public:
    explicit ProcessThread(QObject *parent = nullptr);

protected:
    void run();

signals:
    void outText(int key, QString text);
    void errText(int key, QString text);

public slots:
};

#endif // PROCESSTHREAD_H
