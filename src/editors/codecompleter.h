#ifndef GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
#define GAMS_STUDIO_SYNTAX_CODECOMPLETER_H

#include <QListWidget>

namespace gams {
namespace studio {

class CodeEdit;

class CodeCompleter : public QListWidget
{
    Q_OBJECT

public:
    CodeCompleter(CodeEdit *parent);
    ~CodeCompleter() override;

signals:
    void requestData(QStringList &data);

protected:
    bool event(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    CodeEdit *mEdit;

};


} // namespace studio
} // namespace gams
#endif // GAMS_STUDIO_SYNTAX_CODECOMPLETER_H
