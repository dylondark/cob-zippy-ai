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

    // Q_INVOKABLE makes this function callable from QML
    Q_INVOKABLE bool saveTicket(const QString &message) {

        // Locate the standard "Documents" folder on the OS
        QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QDir dir(path);

        // Safety check: ensure directory exists (it usually does)
        if (!dir.exists()) dir.mkpath(".");

        // Define the file name
        QFile file(path + "/student_tickets.csv");

        // Open in "Append" mode (WriteOnly | Append | Text)
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            qWarning() << "Could not open file for writing:" << file.errorString();
            return false;
        }

        QTextStream out(&file);

        // Generate a timestamp
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        // Write CSV format: "Timestamp","Message"
        // Adding quotes handles cases where the student types a comma in their message
        out << "\"" << timestamp << "\",\"" << message << "\"\n";

        file.close();
        return true;
    }
};

#endif // CONTACTSUPPORT_H
