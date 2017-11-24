#include "application.h"
#include "exception.h"

namespace gams {
namespace studio {

Application::Application(int& argc, char** argv): QApplication(argc, argv)
{}

void Application::showBox(QString title, QString message) {
    QMessageBox box;
    box.setWindowTitle(title);
    box.setText(message);
    box.exec();
}

bool Application::notify(QObject* object, QEvent* event)
{
    try {
        return QApplication::notify(object, event);
    } catch (FatalException &e) {
        showBox(tr("fatal exception"), e.what());
        e.raise();
    } catch (Exception &e) {
        showBox(tr("error"), e.what());
    } catch (QException &e) {
        showBox(tr("external exception"), e.what());
        e.raise();
    } catch (std::exception &e) {
        QString title(tr("standard exception"));
        showBox(title, e.what());
        FATAL() << title << " - " << e.what();
    } catch (...) {
        QString msg(tr("An exception occured. Due to its unknown type the message can't be shown"));
        showBox(tr("unknown exception"), msg);
        FATAL() << msg;
    }
    return true;
}

} // namespace studio
} // namespace gams
