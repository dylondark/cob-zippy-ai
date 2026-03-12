#include "programcontroller.h"
#include <iostream>
#include <QDate>

ProgramController::ProgramController(QObject *parent)
    : QObject(parent),
    settings("cob_zippy_ai.ini", QSettings::IniFormat),
    ollama(settings.value("Ollama/URL", "http://cobgpu1.uanet.edu:11434").toString().toStdString(),
           settings.value("Ollama/Model", "gpt-oss:20b").toString().toStdString(),
           settings.value("Ollama/ContextSize", 32000).toInt(),
           settings.value("Ollama/Timeout", 120).toInt()),
    currentGenerateStatus(Idle)
{
    connect(&ollama, &OllamaInterface::responseReceived, this, &ProgramController::onGenerateFinished);
    connect(&ollama, &OllamaInterface::responseFinished, this, &ProgramController::onStreamFinished);
}

/*
    Sets the URL of the Ollama server.
*/
void ProgramController::setURL(QString url)
{
    ollama.setURL(url.toStdString());
    settings.setValue("Ollama/URL", url);
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
    settings.setValue("Ollama/Model", model);
}

/*
    Returns the model to use.
*/
QString ProgramController::getModel() const
{
    return QString::fromStdString(ollama.getModel());
}

/*
    Sets the context size in tokens.
*/
void ProgramController::setContextSize(int tokens)
{
    ollama.setContextSize(tokens);
    settings.setValue("Ollama/ContextSize", tokens);
}

/*
    Returns the context size in tokens.
*/
int ProgramController::getContextSize() const
{
    return ollama.getContextSize();
}

/*
    Sets the timeout in seconds.
*/
void ProgramController::setTimeout(int seconds)
{
    ollama.setTimeout(seconds);
    settings.setValue("Ollama/Timeout", seconds);
}

/*
    Returns the timeout in seconds.
*/
int ProgramController::getTimeout() const
{
    return ollama.getTimeout();
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
    if (!ollama.isConnected())
    {
        setGenerateStatus(Error);
        return;
    }
    QString systemPrompt = R"(You are Zippy, a helpful AI assistant for the University of Akron College of Business.
Today's date is )" + QDate::currentDate().toString("MMMM d, yyyy") + R"(.

CRITICAL INSTRUCTIONS FOR WEB SEARCH:
1. When looking for events, news, or schedules, you MUST append the current month and year to your web_search query (e.g., "College of Business events March 2026").
2. Before answering, check the dates inside the web search results. If the results are from a past year (like 2024 or 2025), DO NOT present them to the user. State that you could not find current events.

Help users as much as you can with the information you know about the College.
If you are not sure about something, you have access to web search and web fetch tools to allow you to retrieve information from the internet.
Also note that you will be provided with the responses from prior uses of the tools.
If previous responses do not contain the information you are looking for, feel free to use the tools again.
If after using these tools you still cannot find the answer to a question, say you do not know and suggest the user to contact the College directly.

For navigation questions (when someone asks "Where is room X?" or "How do I get to room Y?"), use the get_navigation tool. This tool provides turn-by-turn directions for rooms in the College of Business building Floor 1.

IMPORTANT: Respond directly without showing your thinking process. Do not use <think> tags. Just provide clear, direct answers.

OTHER INFORMATION FOR YOUR REFERENCE:
- The phone number for the College of Business is 330-972-7042 for the undergraduate office and 330-972-7043 for the graduate office.
- The current dean of the College of Business is Dr. Terry Daugherty.
- The main website for the College of Business is https://www.uakron.edu/cba/
)";
    
    ollama.sendPrompt(systemPrompt, prompt);
    setGenerateStatus(Generating);
}

/*
    Slot to be called when Ollama finishes generating a response.
    Decodes the output and then invokes abc2midi to convert the output to a MIDI file.
*/
void ProgramController::onGenerateFinished(QString response)
{
    // connect this in QML to get the response
    emit generateFinished(response);
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
}
void ProgramController::setGenerateStatus(GenerateStatus newStatus)
{
    if (currentGenerateStatus != newStatus)
    {
        currentGenerateStatus = newStatus;
        emit generateStatusChanged();
    }

}
