#ifndef CONTACTSUPPORT_H
#define CONTACTSUPPORT_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

class ContactSupport : public QObject
{
    Q_OBJECT
public:
    explicit ContactSupport(QObject *parent = nullptr) : QObject(parent) {}


    Q_INVOKABLE bool saveTicket(const QString &who, const QString &message) {

        QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QDir dir(path);
        if (!dir.exists()) dir.mkpath(".");

        QFile file(path + "/student_tickets.csv");

        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            qWarning() << "Could not open file for writing:" << file.errorString();
            return false;
        }

        QTextStream out(&file);

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        // If 'who' is empty, it will be empty ( "" )
        out << "\"" << timestamp << "\",\"" << who << "\",\"" << message << "\"\n";

        file.close();
        return true;
    }
};

#endif // CONTACTSUPPORT_H
