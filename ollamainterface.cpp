#include "ollamainterface.h"
#include <iostream>
#include <QEventLoop>

OllamaInterface::OllamaInterface(string url, string model)
    : connected(false), url(std::move(url)), model(std::move(model))
{
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &OllamaInterface::onPingReply);

    worker.moveToThread(&requestThread);
    requestThread.start();
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

    // Use an event loop to wait synchronously for completion (like curl_easy_perform)
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool success = (reply->error() == QNetworkReply::NoError);
    connected = success;
    reply->deleteLater();

    emit pingFinished(success);
    return success;
}

void OllamaInterface::onPingReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        connected = true;
        std::cout << "Ping successful." << std::endl;
    }
    else
    {
        connected = false;
        std::cerr << "Ping failed: " << reply->errorString().toStdString() << std::endl;
    }
    reply->deleteLater();
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
