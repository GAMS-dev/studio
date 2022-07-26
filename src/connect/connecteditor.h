#ifndef CONNECTEDITOR_H
#define CONNECTEDITOR_H

#include "abstractview.h"
#include "connect/connect.h"

namespace gams {
namespace studio {

class MainWindow;

namespace connect {

namespace Ui {
class ConnectEditor;
}

class Connect;

class ConnectEditor : public AbstractView
{
    Q_OBJECT

public:
    explicit ConnectEditor(const QString& connectDataFileName, QWidget *parent = nullptr);
    ~ConnectEditor() override;

private slots:
    void schemaClicked(const QModelIndex &modelIndex);
    void schemaDoubleClicked(const QModelIndex &modelIndex);

private:
    Ui::ConnectEditor *ui;

    bool init();

    Connect* mConnect;
    QString mLocation;
};

}
}
}

#endif // CONNECTEDITOR_H
