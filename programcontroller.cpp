#include "programcontroller.h"
#include <iostream>

ProgramController::ProgramController(QObject *parent)
    : QObject(parent), ollama("http://localhost:11434", "gemma3:4b"), currentGenerateStatus(Error)
{
    connect(&ollama, &OllamaInterface::responseReceived, this, &ProgramController::onGenerateFinished);
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
    QString systemPrompt = "You are Zippy, a helpful AI assistant for the University of Akron College of Business."
                           "Help users as much as you can with the information you know about the College."
                           "If you are not sure about something, say you don't know and suggest they contact the College directly.";

    ollama.sendPrompt(systemPrompt, prompt);
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

void ProgramController::setGenerateStatus(GenerateStatus newStatus)
{
    if (currentGenerateStatus != newStatus)
    {
        currentGenerateStatus = newStatus;
        emit generateStatusChanged();
    }
}
