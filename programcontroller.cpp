#include "programcontroller.h"
#include <QDateTime>
#include <QRegularExpression>
#include <QJsonArray>
#include <QJsonObject>
#include <iostream>

ProgramController::ProgramController(QObject *parent)
    : QObject(parent), ollama("http://localhost:11434", "gemma3:1b"), currentGenerateStatus(Idle), isSearchEnabled(true)
{
    connect(&ollama, &OllamaInterface::responseReceived, this, &ProgramController::onGenerateFinished);
    connect(&ollama, &OllamaInterface::responseFinished, this, &ProgramController::onStreamFinished);
    connect(&ollama, &OllamaInterface::requestError, this, &ProgramController::onRequestError);

    connect(&webSearch, &WebSearchInterface::searchFinished, this, &ProgramController::onSearchFinished);
    connect(&webSearch, &WebSearchInterface::searchError, this, &ProgramController::onSearchError);
}

/*
    Sets the URL of the Ollama server.
*/
void ProgramController::setURL(QString url)
{
    ollama.setURL(url.toStdString());
}

/*
    Returns the URL of the Ollama server.
*/
QString ProgramController::getURL() const
{
    return QString::fromStdString(ollama.getURL());
}

/*
    Sets the model to use.
*/
void ProgramController::setModel(QString model)
{
    ollama.setModel(model.toStdString());
}

/*
    Returns the model to use.
*/
QString ProgramController::getModel() const
{
    return QString::fromStdString(ollama.getModel());
}

/*
    Pings the Ollama server and returns the status.
*/
bool ProgramController::pingOllama()
{
    return ollama.ping();
}

/*
    Returns whether the Ollama server is connected.
*/
bool ProgramController::getOllamaStatus()
{
    return ollama.isConnected();
}

/*
    Prompt the model and begin waiting on response.
*/
void ProgramController::generate(const QString& prompt)
{
    setGenerateStatus(Generating);
    currentPrompt = prompt;
    currentResponse.clear(); // Clear accumulated response from previous generation

    // Add user message to conversation history
    conversationHistory.append(qMakePair(QString("user"), prompt));

    // Priority 1: Try to find answer in local knowledge base
    QString localKnowledgeResult = localKnowledge.search(prompt);

    if (!localKnowledgeResult.isEmpty())
    {
        // We found relevant information in local knowledge base
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentDate = currentDateTime.toString("MMMM d, yyyy");
        QString currentDayOfWeek = currentDateTime.toString("dddd");

        QString systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business. "
                               "IMPORTANT: Today's date is " + currentDayOfWeek + ", " + currentDate + ".\n\n"
                               "LOCAL KNOWLEDGE BASE:\n" + localKnowledgeResult + "\n\n"
                               "CRITICAL RULES YOU MUST FOLLOW:\n"
                               "1. ONLY use information from the LOCAL KNOWLEDGE BASE above\n"
                               "2. NEVER make up names, dates, events, or facts not in the knowledge base\n"
                               "3. NEVER provide URLs or links unless they are in the LOCAL KNOWLEDGE BASE\n"
                               "4. If the knowledge base doesn't contain the answer, say: "
                               "\"I don't have that specific information. Please contact the University of Akron College of Business directly or visit their official website.\"\n\n"
                               "5. Pay attention to notes in the knowledge base about verifying information\n\n"
                               "FORMATTING RULES:\n"
                               "- Use HTML tags for formatting: <b>text</b> for bold, <i>text</i> for italic\n"
                               "- For bullet points, use: <br>• Item 1<br>• Item 2\n"
                               "- Use <br> for line breaks between paragraphs\n"
                               "- Never use markdown (no **, -, #, etc.)";

        // Build messages array from conversation history
        QJsonArray messages;
        for (const auto& historyItem : conversationHistory)
        {
            QJsonObject message;
            message["role"] = historyItem.first;
            message["content"] = historyItem.second;
            messages.append(message);
        }

        ollama.sendChat(systemPrompt, messages);
        return;
    }

    // Priority 2: Check if we should perform a web search
    if (isSearchEnabled && shouldSearch(prompt))
    {
        emit searchingWeb(prompt);
        webSearch.search(prompt);
    }
    else
    {
        // Priority 3: No local knowledge and no web search needed - use general knowledge
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentDate = currentDateTime.toString("MMMM d, yyyy");
        QString currentDayOfWeek = currentDateTime.toString("dddd");

        QString systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business. "
                               "IMPORTANT: Today's date is " + currentDayOfWeek + ", " + currentDate + ". "
                               "Use this current date when answering any time-related questions.\n\n"
                               "CRITICAL RULES YOU MUST FOLLOW:\n"
                               "1. NEVER make up names, dates, events, or facts\n"
                               "2. NEVER provide URLs or links unless they were explicitly provided to you\n"
                               "3. NEVER invent information about people (deans, professors, staff)\n"
                               "4. If you do NOT have verified information in your training data, you MUST say: "
                               "\"I don't have that information. Please contact the University of Akron College of Business directly or visit their official website.\"\n\n"
                               "5. DO NOT guess or speculate - it is better to admit you don't know\n\n"
                               "Examples of correct responses:\n"
                               "- \"I don't know who the current dean is. Please check the University of Akron College of Business website for current staff information.\"\n"
                               "- \"I don't have information about specific events. Please contact the College of Business directly.\"\n\n"
                               "FORMATTING RULES:\n"
                               "- Use HTML tags for formatting: <b>text</b> for bold, <i>text</i> for italic\n"
                               "- For bullet points, use: <br>• Item 1<br>• Item 2\n"
                               "- Use <br> for line breaks between paragraphs\n"
                               "- Structure your response with clear sections and spacing\n"
                               "- Never use markdown (no **, -, #, etc.)\n"
                               "- NEVER include URLs or website links unless they were provided in the conversation";

        // Build messages array from conversation history
        QJsonArray messages;
        for (const auto& historyItem : conversationHistory)
        {
            QJsonObject message;
            message["role"] = historyItem.first;
            message["content"] = historyItem.second;
            messages.append(message);
        }

        ollama.sendChat(systemPrompt, messages);
    }
}

bool ProgramController::shouldSearch(const QString& prompt)
{
    QString lowerPrompt = prompt.toLower();

    // Keywords that suggest the user wants current/live information
    QStringList searchKeywords = {
        "event", "happening", "when", "schedule", "today", "tomorrow",
        "weekend", "next week", "this week", "calendar", "date",
        "current", "latest", "recent", "now", "upcoming"
    };

    for (const QString& keyword : searchKeywords)
    {
        if (lowerPrompt.contains(keyword))
        {
            return true;
        }
    }

    return false;
}

/*
    Cancel the current generation.
*/
void ProgramController::cancelGeneration()
{
    ollama.cancelRequest();
    setGenerateStatus(Idle);
}

/*
    Clear the conversation history.
*/
void ProgramController::clearChat()
{
    conversationHistory.clear();
    currentResponse.clear();
    currentPrompt.clear();
}

/*
    Slot to be called when Ollama finishes generating a response.
    Decodes the output and then invokes abc2midi to convert the output to a MIDI file.
*/
void ProgramController::onGenerateFinished(QString response)
{
    // Accumulate the raw response (not formatted) for conversation history
    currentResponse += response;

    // Convert any markdown to HTML before emitting
    QString formattedResponse = convertMarkdownToHtml(response);

    emit generateFinished(formattedResponse);
}

QString ProgramController::convertMarkdownToHtml(const QString& text)
{
    QString result = text;

    // Convert **bold** to <b>bold</b>
    QRegularExpression boldPattern("\\*\\*(.+?)\\*\\*");
    result.replace(boldPattern, "<b>\\1</b>");

    // Convert *italic* to <i>italic</i>
    QRegularExpression italicPattern("\\*(.+?)\\*");
    result.replace(italicPattern, "<i>\\1</i>");

    // Convert markdown bullet points (- item or * item) to HTML with bullet
    QRegularExpression bulletPattern("^[\\-\\*]\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(bulletPattern, "<br>• \\1");

    // Convert numbered lists (1. item, 2. item) to HTML
    QRegularExpression numberedPattern("^(\\d+)\\. (.+)$", QRegularExpression::MultilineOption);
    result.replace(numberedPattern, "<br>\\1. \\2");

    // Convert ### Header to <b>Header</b>
    QRegularExpression headerPattern("^#{1,6}\\s+(.+)$", QRegularExpression::MultilineOption);
    result.replace(headerPattern, "<br><b>\\1</b><br>");

    // Convert double newlines to paragraph breaks
    result.replace("\n\n", "<br><br>");

    // Convert single newlines to breaks (but not if already <br>)
    QRegularExpression singleNewline("(?<!<br>)\\n(?!<br>)");
    result.replace(singleNewline, "<br>");

    return result;
}

ProgramController::GenerateStatus ProgramController::getGenerateStatus() const
{
    return currentGenerateStatus;
}
void ProgramController::onStreamFinished()
{
    // Add the assistant's complete response to conversation history
    if (!currentResponse.isEmpty())
    {
        conversationHistory.append(qMakePair(QString("assistant"), currentResponse));
    }

    setGenerateStatus(Finished);
    // This emits the new signal for QML to hear
    emit streamFinished();
    // Return to Idle state after a brief moment
    setGenerateStatus(Idle);
}
void ProgramController::setGenerateStatus(GenerateStatus newStatus)
{
    if (currentGenerateStatus != newStatus)
    {
        currentGenerateStatus = newStatus;
        emit generateStatusChanged();
    }
}

void ProgramController::onRequestError(QString error)
{
    setGenerateStatus(Error);
    emit promptParserError(error);
    // Return to Idle state after error
    setGenerateStatus(Idle);
}

void ProgramController::onSearchFinished(QString results)
{
    // Now generate a response using the search results as context
    QString systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business. "
                           "CRITICAL: Pay attention to the current date provided below. Use this date when answering questions about 'today', 'this week', 'upcoming', etc. "
                           "Do NOT use any date from your training data. ONLY use the current date provided here.\n\n"
                           "Context Information from Web Search:\n" + results + "\n\n"
                           "CRITICAL RULES YOU MUST FOLLOW:\n"
                           "1. ONLY use information from the Context Information above\n"
                           "2. NEVER make up names, dates, events, or facts\n"
                           "3. NEVER provide URLs or links that are not in the Context Information\n"
                           "4. NEVER invent information about people (deans, professors, staff)\n"
                           "5. If the Context Information doesn't contain the answer, say: "
                           "\"I don't have that information. Please contact the University of Akron College of Business directly or visit their official website.\"\n\n"
                           "6. DO NOT guess or speculate - only use what's in the Context Information\n\n"
                           "FORMATTING RULES:\n"
                           "- Use HTML tags for formatting: <b>text</b> for bold, <i>text</i> for italic\n"
                           "- For bullet points, use: <br>• Item 1<br>• Item 2\n"
                           "- Use <br><br> for line breaks between paragraphs\n"
                           "- Structure your response with clear sections and spacing\n"
                           "- Never use markdown (no **, -, #, etc.)\n"
                           "- Keep responses well-organized and easy to read";

    // Build messages array from conversation history
    QJsonArray messages;
    for (const auto& historyItem : conversationHistory)
    {
        QJsonObject message;
        message["role"] = historyItem.first;
        message["content"] = historyItem.second;
        messages.append(message);
    }

    ollama.sendChat(systemPrompt, messages);
}

void ProgramController::onSearchError(QString error)
{
    // If search fails, just use the model without search results
    QString systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business. "
                           "Note: Web search was unavailable.\n\n"
                           "CRITICAL RULES YOU MUST FOLLOW:\n"
                           "1. NEVER make up names, dates, events, or facts\n"
                           "2. NEVER provide URLs or links\n"
                           "3. NEVER invent information about people (deans, professors, staff)\n"
                           "4. If you do NOT have verified information, you MUST say: "
                           "\"I don't have that information. Please contact the University of Akron College of Business directly or visit their official website.\"\n\n"
                           "5. DO NOT guess or speculate - it is better to admit you don't know\n\n"
                           "Only provide information that you are 100% confident is accurate.";

    // Build messages array from conversation history
    QJsonArray messages;
    for (const auto& historyItem : conversationHistory)
    {
        QJsonObject message;
        message["role"] = historyItem.first;
        message["content"] = historyItem.second;
        messages.append(message);
    }

    ollama.sendChat(systemPrompt, messages);
}
