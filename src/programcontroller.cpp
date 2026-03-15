#include "programcontroller.h"
#include <iostream>

ProgramController::ProgramController(QObject *parent)
    : QObject(parent),
    settings("cob_zippy_ai.ini", QSettings::IniFormat),
    ollama(settings.value("Ollama/URL", "http://cobgpu1.uanet.edu:11434").toString().toStdString(),
           settings.value("Ollama/Model", "qwen2.5:32b").toString().toStdString(),
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
Help users as much as you can with the information you know about the College.
If you are not sure about something, you have access to web search and web fetch tools to allow you to retrieve information from the internet.
Also note that you will be provided with the responses from prior uses of the tools.
If previous responses do not contain the information you are looking for, feel free to use the tools again.
If after using these tools you still cannot find the answer to a question, say you do not know and suggest the user to contact the College directly.

IMPORTANT: The College of Business calendar pages (calendar.uakron.edu and uakron.edu/cba/calendar) are JavaScript-rendered and CANNOT be fetched — do NOT attempt to web_fetch them.
IMPORTANT: For event questions, use web_search with queries like "University of Akron College of Business events 2026" instead.
IMPORTANT: Do NOT use more than 3 tool calls per question. After 3 attempts, respond with whatever information you have found.

For navigation questions (when someone asks "Where is room X?" or "How do I get to room Y?"), use the get_navigation tool. This tool provides turn-by-turn directions for rooms in the College of Business building Floor 1.

IMPORTANT: Respond directly without showing your thinking process. Do not use <think> tags. Just provide clear, direct answers.

CRITICAL: You MUST only state facts that come from your tool results or from the reference information provided below. NEVER fabricate, invent, or guess events, dates, names, or details. If your search results mention specific events, report ONLY those events with the exact details from the results. If your search results do not contain enough information to fully answer the question, say so honestly — do NOT fill in gaps with made-up information. It is far better to say "I could only find the following events" than to fabricate a complete-looking answer.

OTHER INFORMATION FOR YOUR REFERENCE:
- The College of Business website is https://www.uakron.edu/cba/
- The phone number for the College of Business is 330-972-7042 for the undergraduate office and 330-972-7043 for the graduate office.
- The current dean of the College of Business is Dr. Terry Daugherty (as of January 2026).
- The College of Business is AACSB-accredited and has over 36,000 alumni.

UNDERGRADUATE MAJORS offered by the College of Business:
Accounting, Applied AI & Information Systems, Business Administration, Business Data Analytics, Business Management (online, for returning adults), Economics, Financial Management, Financial Planning, Leadership and Human Resources, Marketing, Sales Management, Sport Analytics, Sport Business, Supply Chain & Operations Management.
Accelerated programs: Business Law/JD, Economics/JD.

UNDERGRADUATE MINORS: Supply Chain/Operations Management, Risk Management and Insurance, Professional Selling, Marketing, Information Systems Management, International Business, Leadership and Human Resources, Financial Planning, Finance, Entrepreneurship, Economics, Business Data Analytics.
Minors for non-business majors: Business Administration, Business Essentials for Engineering, Marketing, Entrepreneurship.

CERTIFICATES: Supply Chain Management, Sport Business, Risk Management and Insurance, Professional Selling, Professional Selling for Engineering, Managing People, Health Care Selling, Financial Planning, Esports Business, Entrepreneurship, Data Analytics and Management, Business Data Analytics.

GRADUATE PROGRAMS: Master of Business Administration (MBA, available online and evenings), Master of Science in Management - Business Analytics (MSM-BA, with Information Systems or Supply Chain concentration), Master of Science in Accountancy (MSA, with Professional Accountancy or Accounting Information Systems options), Master of Taxation (M.Tax), Master of Science in Economics (MSE). Joint degree: JD/MBA with School of Law.
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
