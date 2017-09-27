#include "processthread.h"
#include <iostream>
#include <QTime>

ProcessThread::ProcessThread(QObject *parent) : QThread(parent)
{}

void ProcessThread::run()
{
    srand(QTime::currentTime().msecsSinceStartOfDay()+rand());
    int times = (rand() % 20)+10;
    for (int i = 0; i < times; ++i) {
        msleep(rand()%500);
        QString text;
        int rnd = rand() % 4;
        switch (rnd) {
        case 0:
            text = "error: could not find reason.";
            emit errText(1, text);
            std::cerr << text.toStdString() << std::endl;
            break;
        case 1:
            text = "hint: please wait for next line.";
            emit outText(0, text);
            std::cout << text.toStdString() << std::endl;
            break;
        case 2:
            text = "hint: another line dropped.";
            emit outText(0, text);
            std::cout << text.toStdString() << std::endl;
            break;
        case 3:
            text = "hint: here comes a very long line that blows up everything you expected, because this can "
                   "get your output editor into trouble how to handle such a long line that needs to be cut "
                   "off or eventually wrapped around to one or two next lines. But while wrapping one must "
                   "think about how to show the user that this is one line and not two or three.";
            emit outText(0, text);
            std::cout << text.toStdString() << std::endl;
            break;
        default:
            break;
        }
        msleep(rand()%500);
    }
}
