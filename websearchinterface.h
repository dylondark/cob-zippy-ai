#ifndef WEBSEARCHINTERFACE_H
#define WEBSEARCHINTERFACE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

class WebSearchInterface : public QObject
{
    Q_OBJECT
public:
    explicit WebSearchInterface(QObject *parent = nullptr);
    ~WebSearchInterface();

    // Search the web and return results
    void search(const QString &query);

    // Set the Ollama API key
    void setApiKey(const QString &key);

    // Get the current API key
    QString getApiKey() const;

signals:
    void searchFinished(QString results);
    void searchError(QString error);

private slots:
    void onSearchReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *networkManager;
    QString apiKey;
    QString formatResults(const QJsonArray &results);
};

#endif // WEBSEARCHINTERFACE_H
