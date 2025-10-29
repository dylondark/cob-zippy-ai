#include "programcontroller.h"
#include <QDateTime>
#include <QRegularExpression>
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

    // Check if we should perform a web search first
    if (isSearchEnabled && shouldSearch(prompt))
    {
        emit searchingWeb(prompt);
        webSearch.search(prompt);
    }
    else
    {
        // Get current date even for non-search queries
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString currentDate = currentDateTime.toString("MMMM d, yyyy");
        QString currentDayOfWeek = currentDateTime.toString("dddd");

        QString systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business. "
                               "IMPORTANT: Today's date is " + currentDayOfWeek + ", " + currentDate + ". "
                               "Use this current date when answering any time-related questions. "
                               "Help users as much as you can with the information you know about the College. "
                               "If you are not sure about something, say you don't know and suggest they contact the College directly.\n\n"
                               "FORMATTING RULES:\n"
                               "- Use HTML tags for formatting: <b>text</b> for bold, <i>text</i> for italic\n"
                               "- For bullet points, use: <br>• Item 1<br>• Item 2\n"
                               "- Use <br> for line breaks between paragraphs\n"
                               "- Structure your response with clear sections and spacing\n"
                               "- Never use markdown (no **, -, #, etc.)";

        ollama.sendPrompt(systemPrompt, prompt);
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
    Slot to be called when Ollama finishes generating a response.
    Decodes the output and then invokes abc2midi to convert the output to a MIDI file.
*/
void ProgramController::onGenerateFinished(QString response)
{
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
                           "Context Information:\n" + results + "\n\n"
                           "User's Question: " + currentPrompt + "\n\n"
                           "Instructions:\n"
                           "- Use the CURRENT DATE from the context above\n"
                           "- If asked about events or schedules, acknowledge you need more specific information about College of Business events\n"
                           "- Be helpful and suggest checking the official University of Akron College of Business website or calendar\n"
                           "- Provide a helpful and accurate answer based on the context provided\n\n"
                           "FORMATTING RULES:\n"
                           "- Use HTML tags for formatting: <b>text</b> for bold, <i>text</i> for italic\n"
                           "- For bullet points, use: <br>• Item 1<br>• Item 2\n"
                           "- Use <br><br> for line breaks between paragraphs\n"
                           "- Structure your response with clear sections and spacing\n"
                           "- Never use markdown (no **, -, #, etc.)\n"
                           "- Keep responses well-organized and easy to read";

    ollama.sendPrompt(systemPrompt, currentPrompt);
}

void ProgramController::onSearchError(QString error)
{
    // If search fails, just use the model without search results
    QString systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business. "
                           "Help users as much as you can with the information you know about the College. "
                           "Note: Web search was unavailable, so provide the best answer you can with your existing knowledge.";

    ollama.sendPrompt(systemPrompt, currentPrompt);
}
