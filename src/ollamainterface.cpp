#include "ollamainterface.h"
#include <iostream>
#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <qjsonarray.h>
#include <QSettings>
#include <QMap>
#include <QSet>
#include <QFile>
#include <QTextStream>

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

    toolCallCount = 0; // Reset tool call counter for each new user prompt

    QUrl endpoint(QString::fromStdString(url + "/api/chat"));
    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Start working messages from persistent history
    workingMessages = QJsonArray(messageHistory);

    // Add system and user messages to working context only
    if (!systemPrompt.isEmpty())
    {
        QJsonObject sysMsg;
        sysMsg["role"] = "system";
        sysMsg["content"] = systemPrompt;
        workingMessages.append(sysMsg);
    }
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userPrompt;
    workingMessages.append(userMsg);

    // Save the user prompt so we can add it to persistent history later
    currentUserPrompt = userPrompt;

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

    // build the navigation tool JSON object
    QJsonObject navigationTool;
    navigationTool["type"] = "function";
    QJsonObject navigationFunction;
    navigationFunction["name"] = "get_navigation";
    navigationFunction["description"] = "Get turn-by-turn navigation directions to a specific room in the College of Business building Floor 1. Use this when someone asks for directions to a room.";
    QJsonObject navigationParameters;
    navigationParameters["type"] = "object";
    QJsonObject navProperties;
    QJsonObject roomProperty;
    roomProperty["type"] = "string";
    roomProperty["description"] = "The room number (e.g., '125', '147', '120').";
    navProperties["room_number"] = roomProperty;
    navigationParameters["properties"] = navProperties;
    navigationFunction["parameters"] = navigationParameters;
    navigationTool["function"] = navigationFunction;

    // build the get_events tool JSON object
    QJsonObject eventsTool;
    eventsTool["type"] = "function";
    QJsonObject eventsFunction;
    eventsFunction["name"] = "get_events";
    eventsFunction["description"] = "Get upcoming events from the College of Business calendar. Use this tool whenever someone asks about College of Business events, activities, or what's happening.";
    QJsonObject eventsParameters;
    eventsParameters["type"] = "object";
    eventsParameters["properties"] = QJsonObject(); // no parameters needed
    eventsFunction["parameters"] = eventsParameters;
    eventsTool["function"] = eventsFunction;

    // build the final JSON object to send in the request
    QJsonObject json;
    json["model"] = QString::fromStdString(model);
    json["messages"] = workingMessages;
    json["stream"] = true;
    json["tools"] = QJsonArray() << webSearchTool << webFetchTool << navigationTool << eventsTool;

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

    // Add tool response to working context only (not persistent history)
    QJsonObject toolMsg;
    toolMsg["role"] = "tool";
    toolMsg["content"] = toolResponse;
    workingMessages.append(toolMsg);

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

    // build the get_events tool JSON object
    QJsonObject eventsTool;
    eventsTool["type"] = "function";
    QJsonObject eventsFunction;
    eventsFunction["name"] = "get_events";
    eventsFunction["description"] = "Get upcoming events from the College of Business calendar.";
    QJsonObject eventsParameters;
    eventsParameters["type"] = "object";
    eventsParameters["properties"] = QJsonObject();
    eventsFunction["parameters"] = eventsParameters;
    eventsTool["function"] = eventsFunction;

    QJsonObject json;
    json["model"] = QString::fromStdString(model);
    json["messages"] = workingMessages;
    json["stream"] = false;
    json["tools"] = QJsonArray() << webSearchTool << webFetchTool << eventsTool;

    // send the POST request to the ollama server and wait for the reply
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onPromptReply(reply); });
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
    std::cout << "REQUESTING WEB SEARCH WITH QUERY: " << query.toStdString()
              << std::endl;

    // send the POST request to the ollama server and wait for the reply
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { receiveWebSearch(reply); });
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
    std::cout << "REQUESTING WEB FETCH" << std::endl;

    // send the POST request to the ollama server and wait for the reply
    QNetworkReply *reply = networkManager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { receiveWebFetch(reply); });
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
                // Log parse error for debugging but don't show raw data to user
                std::cerr << "JSON parse error: " << parseError.errorString().toStdString()
                          << " at offset " << parseError.offset << std::endl;
                std::cerr << "Problematic data: " << line.toStdString() << std::endl;
                continue;
            }

            QJsonObject obj = jsonResponse.object();

            if (obj.contains("message"))
            {
                QJsonObject messageObj = obj["message"].toObject();
                QString role = messageObj["role"].toString();
                QString content = messageObj["content"].toString();
                QJsonArray tool_calls_arr = messageObj["tool_calls"].toArray();

                // Only use assistant message content
                if (role == "assistant")
                {
                    // Check if there are tool calls - only access array if not empty
                    if (!tool_calls_arr.isEmpty())
                    {
                        toolCallCount++;

                        // Prevent infinite tool call loops
                        if (toolCallCount > 5)
                        {
                            std::cerr << "Tool call limit reached, forcing response." << std::endl;
                            sendToolPrompt("You have reached the maximum number of tool calls. Please respond with the information you have gathered so far.");
                            return;
                        }

                        // Save the assistant's tool-call message to working context only
                        workingMessages.append(messageObj);

                        QJsonObject tool_calls = tool_calls_arr.first().toObject();
                        QJsonObject tool_calls_func = tool_calls["function"].toObject();

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
                                requestWebSearch(query, apiKey);
                            }
                            else if (toolName == "web_fetch")
                            {
                                QString url = toolArgs["url"].toString();
                                requestWebFetch(url, apiKey);
                            }
                            else if (toolName == "get_navigation")
                            {
                                QString roomNumber = toolArgs["room_number"].toString();
                                QString directions = getNavigation(roomNumber);
                                sendToolPrompt(directions);
                            }
                            else if (toolName == "get_events")
                            {
                                requestEvents();
                            }
                            else
                            {
                                std::cerr << "Unknown tool requested: " << toolName.toStdString() << std::endl;
                                sendToolPrompt("Error: tool '" + toolName + "' does not exist. Available tools are: web_search, web_fetch, get_navigation, get_events. Please respond with what you know.");
                            }
                        }
                        // Do NOT emit responseFinished() yet.
                        // Let the tool finish and prompt the model again.
                        return;
                    }
                    else
                    {
                        // No tool calls, just regular message content
                        text = content;
                        pendingMessage += text;
                        emit responseReceived(text);
                    }
                }
            }
            else if (obj.contains("error"))
            {
                // Handle Ollama error responses gracefully
                QString errorMsg = obj["error"].toString();
                std::cerr << "Ollama error: " << errorMsg.toStdString() << std::endl;
                emit requestError("Ollama error: " + errorMsg);
            }
            // Silently ignore other unexpected JSON structures (e.g., status updates)

            if (obj.contains("done") && obj["done"].toBool())
            {
                reply->deleteLater();
                // Save only user question + final assistant response to persistent history
                addMessageToHistory("user", currentUserPrompt);
                addMessageToHistory("assistant", pendingMessage);
                emit responseFinished();
                pendingMessage.clear();
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
        return;
    }

    QByteArray responseData = reply->readAll();
    // need to parse this and make it pretty for the model
    QJsonObject responseJson = QJsonDocument::fromJson(responseData).object();
    QJsonArray results = responseJson["results"].toArray();
    QString text = "RESULTS FROM WEB SEARCH:\n";
    for (const QJsonValue &result : results)
    {
        QJsonObject resultObj = result.toObject();
        QString title = resultObj["title"].toString();
        QString url = resultObj["url"].toString();
        QString content = resultObj["content"].toString();

        text += "Title: " + title + "\n";
        text += "URL: " + url + "\n";
        text += "Content: " + content + "\n\n";
    }
    if (text == "RESULTS FROM WEB SEARCH:\n")
    {
        text += "No results found. Web search may have encountered an error or is not working currently.\n"
            "Tell the user that you were not able to find the information and suggest they contact the University of Akron College of Business directly."
            "Do not continue attempting to use web search to answer this question.";
    }
    std::cout << text.toStdString() << std::endl;
    reply->deleteLater();
    sendToolPrompt(text);
}

void OllamaInterface::receiveWebFetch(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        emit requestError(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    // do we need to parse this and make it pretty for the model? were gonna say no for now
    QString text = QString::fromUtf8(responseData);
    std::cout << "WEB FETCH RESPONSE: \n" << text.toStdString() << std::endl;

    reply->deleteLater();
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

void OllamaInterface::requestEvents()
{
    QUrl endpoint("https://calendar.uakron.edu/live/json/events/group/College%20of%20Business/");
    QNetworkRequest request(endpoint);
    std::cout << "FETCHING COB CALENDAR EVENTS" << std::endl;

    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { receiveEvents(reply); });
}

void OllamaInterface::receiveEvents(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        std::cerr << "Events fetch error: " << reply->errorString().toStdString() << std::endl;
        sendToolPrompt("Could not fetch events from the College of Business calendar. Suggest the user visit https://calendar.uakron.edu/cba/ directly.");
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    QString text = "UPCOMING COLLEGE OF BUSINESS EVENTS:\n\n";

    if (doc.isArray())
    {
        QJsonArray events = doc.array();
        if (events.isEmpty())
        {
            text += "No upcoming events found on the College of Business calendar.\n";
        }
        else
        {
            for (const QJsonValue &val : events)
            {
                QJsonObject event = val.toObject();
                QString title = event["title"].toString();
                QString date = event["date"].toString();
                QString date2 = event["date2"].toString();
                QString time = event["date_time"].toString();
                QString location = event["location"].toString();
                QString summary = event["summary"].toString();
                QString url = event["url"].toString();

                text += "- " + title + "\n";
                if (!date.isEmpty())
                    text += "  Date: " + date + "\n";
                if (!date2.isEmpty() && date2 != date)
                    text += "  End Date: " + date2 + "\n";
                if (!time.isEmpty())
                    text += "  Time: " + time + "\n";
                if (!location.isEmpty())
                    text += "  Location: " + location + "\n";
                if (!summary.isEmpty())
                    text += "  Details: " + summary + "\n";
                if (!url.isEmpty())
                    text += "  Link: https://calendar.uakron.edu" + url + "\n";
                text += "\n";
            }
        }
    }
    else
    {
        text += "Could not parse calendar data. The calendar may be temporarily unavailable.\n";
    }

    std::cout << text.toStdString() << std::endl;
    reply->deleteLater();
    sendToolPrompt(text);
}

QString OllamaInterface::getNavigation(const QString &roomNumber)
{
    // List of valid rooms on Floor 1 - ONLY these rooms exist
    QSet<QString> validRooms = {
        "120", "121", "125", "126", "130", "131", "132", "133", "134",
        "139", "142", "143", "144", "145", "146", "147", "148", "149"
    };

    // Check if the requested room exists on Floor 1
    if (!validRooms.contains(roomNumber))
    {
        return QString(
            "I don't have navigation information for room %1. "
            "This room is either not on Floor 1 of the College of Business building, "
            "or it doesn't exist in my navigation database. "
            "I can only provide directions for rooms on Floor 1. "
            "If you need help finding a room on a different floor, "
            "please contact the College of Business office at 330-972-7042."
        ).arg(roomNumber);
    }

    // Load navigation map from resource file
    QFile navFile(":/data/floor1_navigation.txt");
    QString navigationMap;

    if (navFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&navFile);
        navigationMap = in.readAll();
        navFile.close();
    }
    else
    {
        std::cerr << "Failed to load navigation data file" << std::endl;
        return QString("Navigation data is currently unavailable. Please try again later.");
    }

    // Build the full prompt with room-specific instructions
    QString fullMap = navigationMap;
    fullMap += "\n\n=== NAVIGATION INSTRUCTIONS ===\n";
    fullMap += "Based on this map, provide turn-by-turn directions to Room " + roomNumber;
    fullMap += " from the default starting position (facing the big screen).\n";
    fullMap += "- Give clear left/right/straight directions\n";
    fullMap += "- Mention landmarks they'll pass (bathrooms, stairs, other rooms)\n";
    fullMap += "- Confirm what they should see when they arrive\n";

    return fullMap;
}
