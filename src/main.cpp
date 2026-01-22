#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "programcontroller.h"
#include "ContactSupport.h" //  <-- 1. Include the new header

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Your Existing Controller
    ProgramController controller;
    engine.rootContext()->setContextProperty("controller", &controller);

    // --- NEW CODE START ---
    // Create the support backend and expose it to QML as "contactSupport"
    ContactSupport supportBackend;
    engine.rootContext()->setContextProperty("contactSupport", &supportBackend);
    // --- NEW CODE END ---

    engine.loadFromModule("cob_zippy_ai", "Main");

    return app.exec();
}
