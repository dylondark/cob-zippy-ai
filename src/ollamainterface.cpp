#include "ollamainterface.h"
#include <iostream>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <qjsonarray.h>
#include <QSettings>

OllamaInterface::OllamaInterface(string url, string model, int contextSize, int timeout)
    : connected(false), url(url), model(model), contextSize(contextSize), timeout(timeout)
{
    networkManager = new QNetworkAccessManager(this);
}

OllamaInterface::~OllamaInterface()
{
    requestThread.quit();
    requestThread.wait();
    delete networkManager;
}

bool OllamaInterface::ping()
{
    QUrl pingUrl(QString::fromStdString(url + "/ping"));
    QNetworkRequest request(pingUrl);
    QNetworkReply *reply = networkManager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool success = (reply->error() == QNetworkReply::NoError || reply->error() == QNetworkReply::ContentNotFoundError);
    connected = success;
    emit pingFinished(success);

    if (!success)
        emit requestError(reply->errorString());

    reply->deleteLater();
    return success;
}

void OllamaInterface::sendPrompt(const QString &systemPrompt, const QString &userPrompt)
{
    if (!connected)
    {
        emit requestError("Not connected to Ollama server.");
        return;
    }

    QUrl endpoint(QString::fromStdString(url + "/api/chat"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // build the chat message JSON objects
    if (!systemPrompt.isEmpty())
    {
        addMessageToHistory("system", systemPrompt);
    }
    addMessageToHistory("user", userPrompt);

    // build the web search tool JSON object
    QJsonObject webSearchTool;
    webSearchTool["type"] = "function";
    QJsonObject webSearchFunction;
    webSearchFunction["name"] = "web_search";
    webSearchFunction["description"] = "Search the University of Akron website for answers.";
    QJsonObject webSearchParameters;
    webSearchParameters["type"] = "object";
    QJsonObject properties;
    QJsonObject queryProperty;
    queryProperty["type"] = "string";
    queryProperty["description"] = "The search query.";
    properties["query"] = queryProperty;
    webSearchParameters["properties"] = properties;
    webSearchFunction["parameters"] = webSearchParameters;
    webSearchTool["function"] = webSearchFunction;

    // build the web fetch tool JSON object
    QJsonObject webFetchTool;
    webFetchTool["type"] = "function";
    QJsonObject webFetchFunction;
    webFetchFunction["name"] = "web_fetch";
    webFetchFunction["description"] = "Fetch content from a given URL.";
    QJsonObject webFetchParameters;
    webFetchParameters["type"] = "object";
    QJsonObject fetchProperties;
    QJsonObject urlProperty;
    urlProperty["type"] = "string";
    urlProperty["description"] = "The URL to fetch content from.";
    fetchProperties["url"] = urlProperty;
    webFetchParameters["properties"] = fetchProperties;
    webFetchFunction["parameters"] = webFetchParameters;
    webFetchTool["function"] = webFetchFunction;

    // build the final JSON object to send in the request
    QJsonObject json;
    json["model"] = QString::fromStdString(model);
    json["messages"] = messageHistory;
    json["stream"] = true;
    json["tools"] = QJsonArray() << webSearchTool << webFetchTool;
    std::cout << QJsonDocument(json).toJson().toStdString() << std::endl;

    // send the POST request to the ollama server and wait for the reply
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() { onPromptReply(reply); });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { reply->deleteLater(); });
}


void OllamaInterface::sendToolPrompt(const QString &toolResponse)
{
    if (!connected)
    {
        emit requestError("Not connected to Ollama server.");
        return;
    }

    QUrl endpoint(QString::fromStdString(url + "/api/chat"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Add tool response as a user message
    addMessageToHistory("tool", toolResponse);

    // build the web search tool JSON object
    QJsonObject webSearchTool;
    webSearchTool["type"] = "function";
    QJsonObject webSearchFunction;
    webSearchFunction["name"] = "web_search";
    webSearchFunction["description"] = "Search the University of Akron website for answers.";
    QJsonObject webSearchParameters;
    webSearchParameters["type"] = "object";
    QJsonObject properties;
    QJsonObject queryProperty;
    queryProperty["type"] = "string";
    queryProperty["description"] = "The search query.";
    properties["query"] = queryProperty;
    webSearchParameters["properties"] = properties;
    webSearchFunction["parameters"] = webSearchParameters;
    webSearchTool["function"] = webSearchFunction;

    // build the web fetch tool JSON object
    QJsonObject webFetchTool;
    webFetchTool["type"] = "function";
    QJsonObject webFetchFunction;
    webFetchFunction["name"] = "web_fetch";
    webFetchFunction["description"] = "Fetch content from a given URL.";
    QJsonObject webFetchParameters;
    webFetchParameters["type"] = "object";
    QJsonObject fetchProperties;
    QJsonObject urlProperty;
    urlProperty["type"] = "string";
    urlProperty["description"] = "The URL to fetch content from.";
    fetchProperties["url"] = urlProperty;
    webFetchParameters["properties"] = fetchProperties;
    webFetchFunction["parameters"] = webFetchParameters;
    webFetchTool["function"] = webFetchFunction;

    QJsonObject json;
    json["model"] = QString::fromStdString(model);
    json["messages"] = messageHistory;
    json["stream"] = false;
    json["tools"] = QJsonArray() << webSearchTool << webFetchTool;

    // send the POST request to the ollama server and wait for the reply
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() { onPromptReply(reply); });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { reply->deleteLater(); });
}

void OllamaInterface::requestWebSearch(const QString &query, const QString &apiKey)
{
    if (!connected)
    {
        emit requestError("Not connected to Ollama server.");
        return;
    }

    QUrl endpoint("https://ollama.com/api/web_search");
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // build api key header
    QString bearerKey = "Bearer " + apiKey;
    request.setRawHeader("Authorization", bearerKey.toUtf8());

    // build the final JSON object to send in the request
    QJsonObject json;
    json["query"] = query;

    // send the POST request to the ollama server and wait for the reply
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() { receiveWebSearch(reply); });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { reply->deleteLater(); });
}

void OllamaInterface::requestWebFetch(const QString &url, const QString &apiKey)
{
    if (!connected)
    {
        emit requestError("Not connected to Ollama server.");
        return;
    }

    QUrl endpoint("https://ollama.com/api/web_fetch");
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // build api key header
    QString bearerKey = "Bearer " + apiKey;
    request.setRawHeader("Authorization", bearerKey.toUtf8());

    // build the final JSON object to send in the request
    QJsonObject json;
    json["url"] = url;

    // send the POST request to the ollama server and wait for the reply
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::readyRead, this, [this, reply]() { receiveWebFetch(reply); });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { reply->deleteLater(); });
}

void OllamaInterface::onPingReply(QNetworkReply *reply)
{
    connected = (reply->error() == QNetworkReply::NoError);

    if (connected)
    {
        std::cout << "Ping successful." << std::endl;
    }
    else
    {
        std::cerr << "Ping failed: " << reply->errorString().toStdString() << std::endl;
        emit requestError(reply->errorString());
    }

    emit pingFinished(connected);
    reply->deleteLater();
}

void OllamaInterface::onPromptReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray responseData = reply->readAll();
        QString text;
        static QString totalMessage;
        std::cout << responseData.toStdString() << std::endl;

        // The response can contain multiple JSON objects separated by newlines
        QList<QByteArray> jsonLines = responseData.split('\n');

        for (const QByteArray &line : jsonLines)
        {
            if (line.trimmed().isEmpty())
                continue;

            QJsonParseError parseError;
            QJsonDocument jsonResponse = QJsonDocument::fromJson(line, &parseError);

            if (parseError.error != QJsonParseError::NoError || !jsonResponse.isObject())
            {
                // If not valid JSON, emit raw data for debugging
                // NOTE: THIS IS TEMPORARY, this should be handled properly
                text = QString::fromUtf8(line);
                emit responseReceived(text);
                continue;
            }

            QJsonObject obj = jsonResponse.object();

            if (obj.contains("message"))
            {
                QJsonObject messageObj = obj["message"].toObject();
                QString role = messageObj["role"].toString();
                QString content = messageObj["content"].toString();
                QJsonArray tool_calls_arr = messageObj["tool_calls"].toArray();
                QJsonObject tool_calls = tool_calls_arr.first().toObject();
                QJsonObject tool_calls_func = tool_calls["function"].toObject();

                // Only use assistant message content
                if (role == "assistant")
                {
                    if (tool_calls_arr.empty())
                    {
                        text = content;
                        totalMessage += text;
                        emit responseReceived(text);
                    }
                    else
                    {
                        // select tool
                        if (tool_calls_func.contains("name") && tool_calls_func.contains("arguments"))
                        {
                            QString toolName = tool_calls_func["name"].toString();
                            QJsonObject toolArgs = tool_calls_func["arguments"].toObject();
                            QSettings settings("cob_zippy_ai.ini", QSettings::IniFormat);
                            QString apiKey = settings.value("API/OllamaKey", "").toString();

                            if (toolName == "web_search")
                            {
                                QString query = toolArgs["query"].toString();
                                //query.append(" site:uakron.edu");
                                requestWebSearch(query, apiKey);
                            }
                            else if (toolName == "web_fetch")
                            {
                                QString url = toolArgs["url"].toString();
                                requestWebFetch(url, apiKey);
                            }
                            else
                            {
                                emit requestError("Unknown tool requested: " + toolName);
                            }
                        }
                    }
                }
            }
            else
            {
                // If something unexpected, emit full JSON line
                // NOTE: THIS IS TEMPORARY, this should be handled properly
                text = QString::fromUtf8(line);
                emit responseReceived(text);
            }

            if (obj.contains("done") && obj["done"].toBool())
            {
                reply->deleteLater();
                addMessageToHistory("assistant", totalMessage);
                emit responseFinished();
                totalMessage.clear();
                return; // Stop processing once done is true
            }
        }
    }
    else
    {
        emit requestError(reply->errorString());
        reply->deleteLater();
    }
}

void OllamaInterface::receiveWebSearch(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit requestError(reply->errorString());
        reply->deleteLater();
    }

    QByteArray responseData = reply->readAll();
    // do we need to parse this and make it pretty for the model? were gonna say no for now
    QString text = QString::fromUtf8(responseData);

    sendToolPrompt(text);
}

void OllamaInterface::receiveWebFetch(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit requestError(reply->errorString());
        reply->deleteLater();
    }

    QByteArray responseData = reply->readAll();
    // do we need to parse this and make it pretty for the model? were gonna say no for now
    QString text = QString::fromUtf8(responseData);
    std::cout << text.toStdString() << std::endl;

    sendToolPrompt(text);
}

bool OllamaInterface::isConnected() const
{
    return connected;
}

void OllamaInterface::setURL(string newUrl)
{
    url = std::move(newUrl);
}

string OllamaInterface::getURL() const
{
    return url;
}

void OllamaInterface::setModel(string newModel)
{
    model = std::move(newModel);
}

string OllamaInterface::getModel() const
{
    return model;
}

void OllamaInterface::setContextSize(int tokens)
{
    contextSize = tokens;
}

int OllamaInterface::getContextSize() const
{
    return contextSize;
}

void OllamaInterface::setTimeout(int seconds)
{
    timeout = seconds;
}

int OllamaInterface::getTimeout() const
{
    return timeout;
}

void OllamaInterface::addMessageToHistory(QString role, QString content)
{
    QJsonObject message;
    message["role"] = role;
    message["content"] = content;
    messageHistory.append(message);
}
