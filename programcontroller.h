#ifndef PROGRAMCONTROLLER_H
#define PROGRAMCONTROLLER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QSettings>
#include <QtQmlIntegration>
#include "ollamainterface.h"

// Structure to hold a single message in the conversation
struct Message {
    QString role;     // "user" or "assistant"
    QString content;  // The message text
};

/*
    Serves as the interface to C++ from QML.
*/
class ProgramController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ProgramController(QObject *parent = nullptr);

    /*
        Sets the URL of the Ollama server.
    */
    Q_INVOKABLE void setURL(QString url);

    /*
        Returns the URL of the Ollama server.
    */
    Q_INVOKABLE QString getURL() const;

    /*
        Sets the model to use.
    */
    Q_INVOKABLE void setModel(QString model);

    /*
        Returns the model to use.
    */
    Q_INVOKABLE QString getModel() const;

    /*
        Pings the Ollama server and returns the status.
    */
    Q_INVOKABLE bool pingOllama();

    /*
        Returns whether the Ollama server is connected.
    */
    Q_INVOKABLE bool getOllamaStatus();

    enum GenerateStatus
    {
        Idle,
        Generating,
        Finished,
        Error
    };
    Q_ENUM(GenerateStatus)

    /*
        Prompt the model and begin waiting on response.
    */
    Q_INVOKABLE void generate(const QString& prompt);

    /*
        Clear the conversation history.
    */
    Q_INVOKABLE void clearConversation();

    Q_INVOKABLE GenerateStatus getGenerateStatus() const;

    Q_PROPERTY(GenerateStatus generateStatus READ getGenerateStatus NOTIFY generateStatusChanged);

signals:
    /*
        Signal to be emitted when Ollama finishes generating a response to pass the response on to QML.
    */
    void generateFinished(QString response);
    void streamFinished();

    /*
        Signal to be emitted when the response from Ollama could not be parsed.
    */
    void promptParserError(QString response);

    /*
        Signal to be emitted when the generate status changes.
    */
    void generateStatusChanged();

    /*
        Signal to be emitted when an error occurs.
    */
    void errorOccurred(QString errorMessage);

private slots:
    /*
        Slot to be called when Ollama finishes generating a response.
        Accumulates streaming chunks.
    */
    void onGenerateFinished(QString response);
    void onStreamFinished();
    void onRequestError(QString error);

private:
    OllamaInterface ollama;

    GenerateStatus currentGenerateStatus;

    // Store conversation history
    QList<Message> conversationHistory;
    QString systemPrompt;

    // Buffer to accumulate the current assistant response
    QString currentResponse;

    // Settings management
    QSettings settings;
    void loadSettings();
    void saveSettings();

    void setGenerateStatus(GenerateStatus);
};

#endif // PROGRAMCONTROLLER_H
