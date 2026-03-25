#include "programcontroller.h"
#include <iostream>
#include <QDateTime>

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
    // Build dynamic date string for system prompt
    QDate today = QDate::currentDate();
    QString todayStr = today.toString("dddd, MMMM d, yyyy");

    QString systemPrompt = QString(R"PROMPT(You are Zippy, a helpful AI assistant for the University of Akron College of Business.
Today's date is %1.
Help users as much as you can with the information you know about the College.
You have access to web search, web fetch, get_events, and get_navigation tools.
IMPORTANT: If the answer is NOT in the reference information provided below, you MUST use your tools to search for it. Do NOT answer from memory or guess — always search first for questions about specific people, locations, directions, buildings, campus facilities, programs, deadlines, tuition, scholarships, or anything not explicitly listed in your reference data below. You do NOT have reliable knowledge about campus locations or directions — ALWAYS search or use get_navigation for these.
Also note that you will be provided with the responses from prior uses of the tools.
If previous responses do not contain the information you are looking for, feel free to use the tools again.
If after using these tools you still cannot find the answer to a question, say you do not know and suggest the user to contact the College directly.

IMPORTANT: The College of Business calendar pages (calendar.uakron.edu and uakron.edu/cba/calendar) are JavaScript-rendered and CANNOT be fetched — do NOT attempt to web_fetch them.
IMPORTANT: For questions about College of Business events, activities, or what's happening, ALWAYS use the get_events tool FIRST. This gives you the official CoB calendar data directly. Only use web_search as a fallback if get_events doesn't have what you need.
IMPORTANT: Do NOT use more than 3 tool calls per question. After 3 attempts, respond with whatever information you have found.

For ANY navigation or location question (where is X, how do I get to X, directions to X), ALWAYS use get_navigation first for rooms in the College of Business building, or web_search for other campus locations. NEVER make up directions from memory.

IMPORTANT: Respond directly without showing your thinking process. Do not use <think> tags. Just provide clear, direct answers.

CRITICAL RULES FOR ACCURACY:
1. You MUST only state facts that come from your tool results or from the reference information provided below. NEVER fabricate, invent, or guess events, dates, names, or details.
2. When reporting events from search results, use the EXACT dates, titles, and details from the results. Do NOT confuse booking deadlines, lodging dates, or registration dates with the actual event date.
3. Pay attention to whether events are in the PAST or FUTURE relative to today's date (%1). Do NOT list past events as upcoming.
4. If your search results do not contain events matching what the user asked for (e.g., "this weekend" but no weekend events found), say so honestly. Do NOT fill in gaps with made-up information.
5. It is far better to say "I could not find any College of Business events for this weekend" than to fabricate a complete-looking answer.

OTHER INFORMATION FOR YOUR REFERENCE:
- The College of Business website is https://www.uakron.edu/cba/
- The phone number for the College of Business is 330-972-7042 for the undergraduate office and 330-972-7043 for the graduate office.
- The current dean of the College of Business is Dr. Terry Daugherty (as of January 2026).
- The College of Business is AACSB-accredited and has over 36,000 alumni.
- You were created by the "Super Awesome Tech Team over at Room 102". Encourage students to come ask us if they have any questions.
- When asked about monetary questions, default to using the dollar.

UNDERGRADUATE MAJORS offered by the College of Business:
Accounting, Applied AI & Information Systems, Business Administration, Business Data Analytics, Business Management (online, for returning adults), Economics, Financial Management, Financial Planning, Leadership and Human Resources, Marketing, Sales Management, Sport Analytics, Sport Business, Supply Chain & Operations Management.
Accelerated programs: Business Law/JD, Economics/JD.

UNDERGRADUATE MINORS: Supply Chain/Operations Management, Risk Management and Insurance, Professional Selling, Marketing, Information Systems Management, International Business, Leadership and Human Resources, Financial Planning, Finance, Entrepreneurship, Economics, Business Data Analytics.
Minors for non-business majors: Business Administration, Business Essentials for Engineering, Marketing, Entrepreneurship.

CERTIFICATES: Supply Chain Management, Sport Business, Risk Management and Insurance, Professional Selling, Professional Selling for Engineering, Managing People, Health Care Selling, Financial Planning, Esports Business, Entrepreneurship, Data Analytics and Management, Business Data Analytics.

GRADUATE PROGRAMS: Master of Business Administration (MBA, available online and evenings), Master of Science in Management - Business Analytics (MSM-BA, with Information Systems or Supply Chain concentration), Master of Science in Accountancy (MSA, with Professional Accountancy or Accounting Information Systems options), Master of Taxation (M.Tax), Master of Science in Economics (MSE). Joint degree: JD/MBA with School of Law.
)PROMPT").arg(todayStr);
    
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
void ProgramController::abortGeneration()
{
    ollama.abortGeneration();
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
