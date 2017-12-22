#include "application.h"
#include "exception.h"

namespace gams {
namespace studio {

Application::Application(int& argc, char** argv): QApplication(argc, argv)
{}

void Application::showBox(QString title, QString message) {
    QMessageBox box;
    box.setWindowTitle(title);
//    QStringList parts = message.split("> ");
//    if (parts.count()>1) {
//        parts.removeAt(0);
//        box.setText(parts.join("> "));
//    } else
        box.setText(message);
    box.exec();
}

bool Application::notify(QObject* object, QEvent* event)
{
    try {
        return QApplication::notify(object, event);
    } catch (FatalException &e) {
        Application::showBox(tr("fatal exception"), e.what());
        e.raise();
    } catch (Exception &e) {
        Application::showBox(tr("error"), e.what());
    } catch (QException &e) {
        Application::showBox(tr("external exception"), e.what());
        e.raise();
    } catch (std::exception &e) {
        QString title(tr("standard exception"));
        Application::showBox(title, e.what());
        FATAL() << title << " - " << e.what();
    } catch (...) {
        QString msg(tr("An exception occured. Due to its unknown type the message can't be shown"));
        Application::showBox(tr("unknown exception"), msg);
        FATAL() << msg;
    }
    return true;
}

} // namespace studio
} // namespace gams
