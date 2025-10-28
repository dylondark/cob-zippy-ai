#include "programcontroller.h"
#include <iostream>

ProgramController::ProgramController(QObject *parent)
    : QObject(parent),
      ollama("http://localhost:11434", "gemma3:4b"),
      currentGenerateStatus(Idle),
      settings("UniversityOfAkron", "ZippyAI")
{
    connect(&ollama, &OllamaInterface::responseReceived, this, &ProgramController::onGenerateFinished);
    connect(&ollama, &OllamaInterface::responseFinished, this, &ProgramController::onStreamFinished);
    connect(&ollama, &OllamaInterface::requestError, this, &ProgramController::onRequestError);

    // Initialize system prompt
    systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business. "
                   "Help users as much as you can with the information you know about the College. "
                   "If you are not sure about something, say you don't know and suggest they contact the College directly.";

    // Load saved settings
    loadSettings();
}

/*
    Sets the URL of the Ollama server.
*/
void ProgramController::setURL(QString url)
{
    ollama.setURL(url.toStdString());
    saveSettings();
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
    saveSettings();
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
    // Add user message to history
    Message userMsg;
    userMsg.role = "user";
    userMsg.content = prompt;
    conversationHistory.append(userMsg);

    // Clear the current response buffer
    currentResponse.clear();

    // Convert Message to OllamaMessage format
    QList<OllamaMessage> ollamaMessages;
    for (const auto& msg : conversationHistory)
    {
        OllamaMessage ollamaMsg;
        ollamaMsg.role = msg.role;
        ollamaMsg.content = msg.content;
        ollamaMessages.append(ollamaMsg);
    }

    // Send chat with full conversation history
    setGenerateStatus(Generating);
    ollama.sendChat(systemPrompt, ollamaMessages);
}

/*
    Clear the conversation history.
*/
void ProgramController::clearConversation()
{
    conversationHistory.clear();
    currentResponse.clear();
}

/*
    Slot to be called when Ollama finishes generating a response.
    Accumulates streaming chunks.
*/
void ProgramController::onGenerateFinished(QString response)
{
    // Accumulate the response
    currentResponse += response;

    // Forward to QML for display
    emit generateFinished(response);
}

ProgramController::GenerateStatus ProgramController::getGenerateStatus() const
{
    return currentGenerateStatus;
}
void ProgramController::onStreamFinished()
{
    // Add the complete assistant response to conversation history
    if (!currentResponse.isEmpty())
    {
        Message assistantMsg;
        assistantMsg.role = "assistant";
        assistantMsg.content = currentResponse;
        conversationHistory.append(assistantMsg);
    }

    // Update status
    setGenerateStatus(Finished);

    // Emit signal for QML
    emit streamFinished();

    // Reset to idle after a moment
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

void ProgramController::loadSettings()
{
    // Load URL and model from settings
    QString savedUrl = settings.value("ollama/url", "http://localhost:11434").toString();
    QString savedModel = settings.value("ollama/model", "gemma3:4b").toString();

    ollama.setURL(savedUrl.toStdString());
    ollama.setModel(savedModel.toStdString());

    std::cout << "Loaded settings - URL: " << savedUrl.toStdString()
              << ", Model: " << savedModel.toStdString() << std::endl;
}

void ProgramController::saveSettings()
{
    // Save current URL and model
    settings.setValue("ollama/url", QString::fromStdString(ollama.getURL()));
    settings.setValue("ollama/model", QString::fromStdString(ollama.getModel()));
    settings.sync();

    std::cout << "Settings saved" << std::endl;
}

void ProgramController::onRequestError(QString error)
{
    std::cerr << "Error: " << error.toStdString() << std::endl;
    setGenerateStatus(Error);
    emit errorOccurred(error);
}
