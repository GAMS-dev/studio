#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QString>
#include <stdexcept>

#ifdef _WIN64
#include <windows.h>
#include <dbghelp.h>
#include <tchar.h>
#endif

// Crash functions
void nullPointerCrash() {
    int* ptr = nullptr;
    *ptr = 42;
}

void divideByZeroCrash() {
    int x = 1;
    int y = 0;
    int z = x / y;
    (void)z;
}

void segmentationFaultCrash() {
    char* ptr = (char*)0xCCCCCCCC;
    *ptr = 'X';
}

void stackOverflowCrash() {
    stackOverflowCrash();
}

void uncaughtExceptionCrash() {
    throw std::runtime_error("This is an uncaught exception!");
}

void accessQListRange() {
    QList<int> x;
    x << 1;
    qDebug() << x[1];
}

// Handle crash by string
void handleCrash(const QString& crashType)
{
    if (crashType == "null") {
        nullPointerCrash();
    } else if (crashType == "div0") {
        divideByZeroCrash();
    } else if (crashType == "segfault") {
        segmentationFaultCrash();
    } else if (crashType == "stack") {
        stackOverflowCrash();
    } else if (crashType == "exception") {
        uncaughtExceptionCrash();
    } else if (crashType == "qlist") {
        accessQListRange();
    } else {
        qDebug() << "Unknown crash type:" << crashType.toStdString();
        exit(1);
    }
}

#ifdef _WIN64
LONG WINAPI crashHandler(EXCEPTION_POINTERS* exceptionInfo)
{
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();
    CONTEXT context = *exceptionInfo->ContextRecord;

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    DWORD64 base = SymLoadModuleEx(process, NULL, exePath, NULL, 0, 0, NULL, 0);
    if (base == 0)
        qDebug() << "SymLoadModuleEx failed:" << GetLastError();

    SymInitialize(process, NULL, TRUE);  // Load symbols
    DWORD imageType = IMAGE_FILE_MACHINE_AMD64;
    STACKFRAME64 stack = {};
    stack.AddrPC.Offset = context.Rip;
    stack.AddrPC.Mode = AddrModeFlat;
    stack.AddrFrame.Offset = context.Rsp;
    stack.AddrFrame.Mode = AddrModeFlat;
    stack.AddrStack.Offset = context.Rsp;
    stack.AddrStack.Mode = AddrModeFlat;

    qDebug() << "\n=== Stack Trace ===\n";
    for (int frame = 0; frame < 40; ++frame) {
        if (!StackWalk64(imageType, process, thread, &stack, &context, NULL,
                         SymFunctionTableAccess64, SymGetModuleBase64, NULL))
            break;

        if (stack.AddrPC.Offset == 0)
            break;

        BYTE symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        IMAGEHLP_LINE64 line;
        DWORD displacementLine = 0;
        ZeroMemory(&line, sizeof(IMAGEHLP_LINE64));
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD64 displacement = 0;
        if (SymGetLineFromAddr64(process, stack.AddrPC.Offset, &displacementLine, &line))
            qDebug() << "L" << frame << ":" << symbol->Name << " at" << line.FileName << ":" << line.LineNumber;
        else if (SymFromAddr(process, stack.AddrPC.Offset, &displacement, symbol))
            qDebug() << "S"  << frame << ":" << symbol->Name << "[ 0x" << Qt::hex << symbol->Address << "]" << Qt::dec;
        else
            qDebug() << "X"  << frame << ": [ 0x" << Qt::hex << stack.AddrPC.Offset << "] (no symbol)";
    }

    SymCleanup(process);
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

#ifdef _WIN64
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
        qDebug() << "SymInitialize failed";
    SetUnhandledExceptionFilter(crashHandler);  // register global exception handler
#endif

    // Argument mode
    try {
        if (argc > 1) {
            QString arg = argv[1];
            handleCrash(arg);
            return 0;
            qDebug() << "The application crashed (direct call)";
        }

        // GUI mode
        QWidget window;
        window.setWindowTitle("Qt Crash Demo");

        QVBoxLayout* layout = new QVBoxLayout;

        auto addButton = [&](const QString& label, auto func) {
            QPushButton* button = new QPushButton(label);
            QObject::connect(button, &QPushButton::clicked, func);
            layout->addWidget(button);
        };

        addButton("Null Pointer Dereference", nullPointerCrash);
        addButton("Divide by Zero", divideByZeroCrash);
        addButton("Segmentation Fault", segmentationFaultCrash);
        addButton("Stack Overflow", stackOverflowCrash);
        addButton("Throw Uncaught Exception", uncaughtExceptionCrash);
        addButton("Access QList out of range", accessQListRange);

        window.setLayout(layout);
        window.show();
        return app.exec();

    } catch (std::exception &e) {
        qDebug() << "Exception thrown: " << e.what();
        QMessageBox::critical(nullptr, "Crash", e.what());
    } catch (...) {
        qDebug() << "The application crashed";
        QMessageBox::critical(nullptr, "Crash", "Unspecified crash");
    }

}
