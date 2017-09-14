#ifndef EDITOR_H
#define EDITOR_H

#include <QWidget>

namespace Ui {
class Editor;
}

class Editor : public QWidget
{
    Q_OBJECT

public:
    explicit Editor(QWidget *parent = 0);
    ~Editor();

private:
    Ui::Editor *ui;
};

#endif // EDITOR_H
