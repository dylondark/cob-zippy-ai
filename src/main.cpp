#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "programcontroller.h"
#include "ContactSupport.h"

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

    ProgramController controller;
    engine.rootContext()->setContextProperty("controller", &controller);
    ContactSupport supportBackend;
    engine.rootContext()->setContextProperty("contactSupport", &supportBackend);

    engine.loadFromModule("cob_zippy_ai", "Main");
    return app.exec();
}
