#include "websearchinterface.h"
#include <QUrlQuery>
#include <QUrl>
#include <QDateTime>
#include <iostream>

WebSearchInterface::WebSearchInterface(QObject *parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
}

WebSearchInterface::~WebSearchInterface()
{
    delete networkManager;
}

void WebSearchInterface::search(const QString &query)
{
    // Get current date/time information
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentDate = currentDateTime.toString("MMMM d, yyyy");
    QString currentDayOfWeek = currentDateTime.toString("dddd");

    // Build context-rich search results with current date
    QString searchContext = "Current Date: " + currentDate + " (" + currentDayOfWeek + ")\n";
    searchContext += "Query: " + query + "\n\n";

    // Using DuckDuckGo Instant Answer API (free, no API key required)
    QUrl url("https://api.duckduckgo.com/");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", "University of Akron " + query);
    urlQuery.addQueryItem("format", "json");
    urlQuery.addQueryItem("no_html", "1");
    urlQuery.addQueryItem("skip_disambig", "1");
    url.setQuery(urlQuery);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ZippyAI/1.0");

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, searchContext]() {
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

            // Get the abstract (main answer)
            QString abstract = obj["Abstract"].toString();
            QString abstractText = obj["AbstractText"].toString();
            QString abstractSource = obj["AbstractSource"].toString();
            QString abstractURL = obj["AbstractURL"].toString();

            if (!abstractText.isEmpty())
            {
                results += "Web Search Results:\n";
                results += "Summary: " + abstractText + "\n";
                if (!abstractSource.isEmpty())
                    results += "Source: " + abstractSource + "\n";
                if (!abstractURL.isEmpty())
                    results += "URL: " + abstractURL + "\n";
            }

            // Get related topics
            QJsonArray relatedTopics = obj["RelatedTopics"].toArray();
            if (!relatedTopics.isEmpty() && abstractText.isEmpty())
            {
                results += "Related Information:\n";
                int count = 0;
                for (const QJsonValue &value : relatedTopics)
                {
                    if (count >= 3) break; // Limit to 3 results

                    QJsonObject topic = value.toObject();
                    QString text = topic["Text"].toString();
                    QString firstURL = topic["FirstURL"].toString();

                    if (!text.isEmpty())
                    {
                        results += "- " + text + "\n";
                        if (!firstURL.isEmpty())
                            results += "  URL: " + firstURL + "\n";
                        count++;
                    }
                }
            }

            if (abstractText.isEmpty() && relatedTopics.isEmpty())
            {
                results += "Note: Limited search results available. Using current date context above.\n";
            }

            emit searchFinished(results);
        }
        else
        {
            // Even if search fails, send date context
            results += "Note: Web search returned no specific results. Use the current date above to provide context.\n";
            emit searchFinished(results);
        }
    }
    else
    {
        // Even on error, send date context
        results += "Note: Web search unavailable. Use the current date above to provide context.\n";
        emit searchFinished(results);
    }

    reply->deleteLater();
}
