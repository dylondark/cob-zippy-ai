#include "websearchinterface.h"
#include <QUrlQuery>
#include <QUrl>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <iostream>

WebSearchInterface::WebSearchInterface(QObject *parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);

    // Try to load API key from file
    QFile keyFile("ollama_api_key.txt");
    if (keyFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&keyFile);
        apiKey = in.readLine().trimmed();
        keyFile.close();
    }
}

WebSearchInterface::~WebSearchInterface()
{
    delete networkManager;
}

void WebSearchInterface::setApiKey(const QString &key)
{
    apiKey = key;
}

QString WebSearchInterface::getApiKey() const
{
    return apiKey;
}

void WebSearchInterface::search(const QString &query)
{
    // Check if API key is available
    if (apiKey.isEmpty())
    {
        emit searchError("Ollama API key not configured. Please add your API key to ollama_api_key.txt");
        return;
    }

    // Use Ollama's web search API
    QUrl url("https://ollama.com/api/web_search");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    // Build the JSON request body
    QJsonObject jsonBody;
    jsonBody["query"] = "University of Akron College of Business " + query;
    jsonBody["max_results"] = 5;

    QJsonDocument doc(jsonBody);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onSearchReply(reply);
    });
}

void WebSearchInterface::onSearchReply(QNetworkReply *reply)
{
    // Get current date/time information
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentDate = currentDateTime.toString("MMMM d, yyyy");
    QString currentDayOfWeek = currentDateTime.toString("dddd");
    QString currentTime = currentDateTime.toString("h:mm AP");

    QString results = "IMPORTANT CONTEXT - Current Date & Time:\n";
    results += "Today is " + currentDayOfWeek + ", " + currentDate + " at " + currentTime + "\n\n";

    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (jsonDoc.isObject())
        {
            QJsonObject obj = jsonDoc.object();
            QJsonArray resultsArray = obj["results"].toArray();

            if (!resultsArray.isEmpty())
            {
                results += "Web Search Results:\n\n";

                for (int i = 0; i < resultsArray.size() && i < 5; ++i)
                {
                    QJsonObject result = resultsArray[i].toObject();
                    QString title = result["title"].toString();
                    QString url = result["url"].toString();
                    QString content = result["content"].toString();

                    results += QString("Result %1:\n").arg(i + 1);
                    if (!title.isEmpty())
                        results += "Title: " + title + "\n";
                    if (!content.isEmpty())
                        results += "Content: " + content + "\n";
                    if (!url.isEmpty())
                        results += "URL: " + url + "\n";
                    results += "\n";
                }

                emit searchFinished(results);
            }
            else
            {
                // No results found
                results += "Note: Web search returned no specific results. Using current date context above.\n";
                emit searchFinished(results);
            }
        }
        else
        {
            // Invalid JSON response
            results += "Note: Web search returned invalid response. Using current date context above.\n";
            emit searchFinished(results);
        }
    }
    else
    {
        // Network error - emit the error message
        QString errorMsg = "Web search error: " + reply->errorString();
        if (reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            errorMsg = "Web search authentication failed. Please check your Ollama API key in ollama_api_key.txt";
        }

        // Still provide date context even on error
        results += "Note: " + errorMsg + "\nUsing current date context above.\n";
        emit searchFinished(results);
    }

    reply->deleteLater();
}
