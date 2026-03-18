#ifndef OLLAMAINTERFACE_H
#define OLLAMAINTERFACE_H

#include <QObject>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <string>
#include "threadworker.h"

using std::string;

class OllamaInterface : public QObject
{
    Q_OBJECT
public:
    explicit OllamaInterface(string url, string model, int contextSize, int timeout);
    ~OllamaInterface();

    // Ping the Ollama server
    bool ping();

    // Send a prompt to the model and receive the result asynchronously
    void sendPrompt(const QString &systemPrompt, const QString &userPrompt);

    // request web search from ollama api
    void requestWebSearch(const QString &query, const QString &apiKey);

    // request web fetch from ollama api
    void requestWebFetch(const QString &url, const QString &apiKey);

    // get navigation directions for a room
    QString getNavigation(const QString &roomNumber);

    // fetch upcoming events from the CoB calendar
    void requestEvents();

    bool isConnected() const;
    void setURL(string url);
    string getURL() const;
    void setModel(string model);
    string getModel() const;
    void setContextSize(int tokens);
    int getContextSize() const;
    void setTimeout(int seconds);
    int getTimeout() const;

signals:
    void pingFinished(bool success);
    void responseReceived(const QString &response);
    void responseFinished();
    void requestError(const QString &error);

private slots:
    void onPingReply(QNetworkReply *reply);
    void onPromptReply(QNetworkReply *reply);
    void receiveWebSearch(QNetworkReply *reply);
    void receiveWebFetch(QNetworkReply *reply);
    void receiveEvents(QNetworkReply *reply);

private:
    void addMessageToHistory(QString role, QString content);

    // send a prompt to the model containing the response from a tool function
    void sendToolPrompt(const QString &toolResponse);

    bool connected;
    string url;
    string model;
    int contextSize; // in tokens
    int timeout; // in seconds
    QJsonArray messageHistory;      // Persistent: only user + final assistant messages
    QJsonArray workingMessages;     // Temporary: includes tool calls for current exchange
    QString currentUserPrompt;      // The current user's question (saved for history after response)
    QString pendingMessage;         // Accumulates streaming response content
    int toolCallCount = 0;          // Track tool calls per prompt to prevent infinite loops

    QNetworkAccessManager *networkManager;
    QThread requestThread;
    ThreadWorker worker;
};

#endif // OLLAMAINTERFACE_H
