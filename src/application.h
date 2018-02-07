#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtWidgets>

namespace gams {
namespace studio {

class Application : public QApplication
{
public:
    Application(int &argc, char **argv);

    bool notify(QObject *object, QEvent *event) override;

    ///
    /// \brief Show a <c>QMessageBox::critical</c> message.
    /// \param title Title of the message.
    /// \param message The exception/error message.
    ///
    static void showExceptionMessage(const QString &title, const QString &message);

    ///
    /// \brief Gets the GAMS Studio version.
    /// \return GAMS Studio version.
    ///
    static QString version();
};

} // namespace studio
} // namespace gams

#endif // APPLICATION_H
