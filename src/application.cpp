#include "application.h"
#include "exception.h"

namespace gams {
namespace studio {

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{

}

bool Application::notify(QObject* object, QEvent* event)
{
    try {
        return QApplication::notify(object, event);
    } catch (FatalException &e) {
        Application::showExceptionMessage(tr("fatal exception"), e.what());
        e.raise();
    } catch (Exception &e) {
        Application::showExceptionMessage(tr("error"), e.what());
    } catch (QException &e) {
        Application::showExceptionMessage(tr("external exception"), e.what());
        e.raise();
    } catch (std::exception &e) {
        QString title(tr("standard exception"));
        Application::showExceptionMessage(title, e.what());
        FATAL() << title << " - " << e.what();
    } catch (...) {
        QString msg(tr("An exception occured. Due to its unknown type the message can't be shown"));
        Application::showExceptionMessage(tr("unknown exception"), msg);
        FATAL() << msg;
    }
    return true;
}

void Application::showExceptionMessage(const QString &title, const QString &message) {
    QMessageBox::critical(nullptr, title, message);
}

QString Application::version() {
    return STUDIO_VERSION;
}

} // namespace studio
} // namespace gams
