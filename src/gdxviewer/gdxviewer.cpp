#include "gdxviewer.h"
#include "exception.h"

namespace gams {
namespace studio {
namespace gdxviewer {

GdxViewer::GdxViewer(QString gdxFile, QString systemDirectory, QWidget *parent) :
    QFrame(parent)
{
    ui.setupUi(this);
    char msg[GMS_SSSIZE];
    int errNr = 0;
    if (!gdxCreateD(&mGdx, systemDirectory.toLatin1(), msg, sizeof(msg)))
    {
        //TODO(CW): raise exception wit proper message and remove the cout
        qDebug() << "**** Could not load GDX library";
        qDebug() << "**** " << msg;
        throw Exception();
    }

    gdxOpenRead(mGdx, gdxFile.toLatin1(), &errNr);
    if (errNr) reportIoError(errNr,"gdxOpenRead");

    mGdxSymbols =loadGDXSymbol();
}

void GdxViewer::reportIoError(int errNr, QString message)
{
    //TODO(CW): proper Exception message and remove qDebug
    qDebug() << "**** Fatal I/O Error = " << errNr << " when calling " << message;
    throw Exception();
}

QList<GDXSymbol> GdxViewer::loadGDXSymbol()
{
    int symbolCount = 0;
    int uelCount = 0;
    gdxSystemInfo(mGdx, &symbolCount, &uelCount);
    QList<GDXSymbol> symbols;
    for(int i=1; i<symbolCount+1; i++)
    {
        char symName[GMS_SSSIZE];
        char explText[GMS_SSSIZE];
        int dimension = 0;
        int type = 0;
        gdxSymbolInfo(mGdx, i, symName, &dimension, &type);
        int recordCount = 0;
        int userInfo = 0;
        gdxSymbolInfoX (mGdx, i, &recordCount, &userInfo, explText);
        symbols.append(GDXSymbol(i, QString(symName), dimension, type, userInfo, recordCount, QString(explText)));
    }
    return symbols;
}

} // namespace gdxviewer
} // namespace studio
} // namespace gams
