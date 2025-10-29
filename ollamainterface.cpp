#include "ollamainterface.h"
#include <iostream>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

OllamaInterface::OllamaInterface(string url, string model)
    : connected(false), url(std::move(url)), model(std::move(model)), currentReply(nullptr)
{
    networkManager = new QNetworkAccessManager(this);

    // Create a timer for buffering responses
    bufferTimer = new QTimer(this);
    bufferTimer->setInterval(50); // Flush buffer every 50ms
    bufferTimer->setSingleShot(true);
    connect(bufferTimer, &QTimer::timeout, this, &OllamaInterface::flushBuffer);
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

    // Clear any existing buffer
    responseBuffer.clear();
    bufferTimer->stop();

    QUrl endpoint(QString::fromStdString(url + "/api/generate"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["model"] = QString::fromStdString(model);
    json["system"] = systemPrompt;
    json["prompt"] = userPrompt;
    json["stream"] = true;  // Can be set to true for streaming responses

    currentReply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(currentReply, &QNetworkReply::readyRead, this, [this]() { onPromptReply(currentReply); });
}

void OllamaInterface::cancelRequest()
{
    if (currentReply != nullptr)
    {
        currentReply->abort();
        currentReply->deleteLater();
        currentReply = nullptr;
        emit responseFinished();
    }
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
        QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
        QString text;

        if (jsonResponse.isObject())
        {
            QJsonObject obj = jsonResponse.object();
            if (obj.contains("response"))
                text = obj["response"].toString();
            else
                text = QString::fromUtf8(responseData);

            if (obj.contains("done"))
            {
                bool done = obj["done"].toBool();
                if (done)
                {
                    reply->deleteLater();
                    currentReply = nullptr;
                    // Flush any remaining buffered text
                    if (!responseBuffer.isEmpty())
                    {
                        emit responseReceived(responseBuffer);
                        responseBuffer.clear();
                    }
                    if (!text.isEmpty())
                    {
                        emit responseReceived(text);
                    }
                    emit responseFinished();
                    return; // Finished receiving response
                }
            }
        }
        else
        {
            text = QString::fromUtf8(responseData);
        }

        // Buffer the response text
        responseBuffer += text;

        // Restart the timer - will flush after 50ms of no new data
        bufferTimer->start();
    }
    else
    {
        currentReply = nullptr;
        emit requestError(reply->errorString());
        reply->deleteLater();
    }
}

void OllamaInterface::flushBuffer()
{
    if (!responseBuffer.isEmpty())
    {
        emit responseReceived(responseBuffer);
        responseBuffer.clear();
    }
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
