#ifndef OLLAMAINTERFACE_H
#define OLLAMAINTERFACE_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <string>
#include "threadworker.h"

using std::string;

class OllamaInterface : public QObject
{
    Q_OBJECT
public:
    explicit OllamaInterface(string url, string model);
    ~OllamaInterface();

    // Pings the Ollama server to check if it is available.
    bool ping();

    bool isConnected() const;
    void setURL(string url);
    string getURL() const;
    void setModel(string model);
    string getModel() const;

signals:
    void pingFinished(bool success);

private slots:
    void onPingReply(QNetworkReply *reply);

private:
    bool connected;
    string url;
    string model;

    QNetworkAccessManager *networkManager;
    QThread requestThread;
    ThreadWorker worker;
};

#endif // OLLAMAINTERFACE_H
