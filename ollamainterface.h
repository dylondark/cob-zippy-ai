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

    // Ping the Ollama server
    bool ping();

    // Send a prompt to the model and receive the result asynchronously
    void sendPrompt(const QString &systemPrompt, const QString &userPrompt);

    bool isConnected() const;
    void setURL(string url);
    string getURL() const;
    void setModel(string model);
    string getModel() const;

signals:
    void pingFinished(bool success);
    void responseReceived(const QString &response);
    void responseFinished();
    void requestError(const QString &error);

private slots:
    void onPingReply(QNetworkReply *reply);
    void onPromptReply(QNetworkReply *reply);

private:
    bool connected;
    string url;
    string model;

    QNetworkAccessManager *networkManager;
    QThread requestThread;
    ThreadWorker worker;
};

#endif // OLLAMAINTERFACE_H
