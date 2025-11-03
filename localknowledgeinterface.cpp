#include "localknowledgeinterface.h"
#include <QTextStream>
#include <QRegularExpression>

LocalKnowledgeInterface::LocalKnowledgeInterface(QObject *parent)
    : QObject(parent), loaded(false)
{
    // Try to load the knowledge base on construction
    loadKnowledgeBase("knowledge_base.txt");
}

bool LocalKnowledgeInterface::loadKnowledgeBase(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        loaded = false;
        return false;
    }

    QTextStream in(&file);
    knowledgeBaseContent = in.readAll();
    file.close();

    loaded = !knowledgeBaseContent.isEmpty();
    return loaded;
}

bool LocalKnowledgeInterface::isLoaded() const
{
    return loaded;
}

QString LocalKnowledgeInterface::search(const QString &query)
{
    if (!loaded || knowledgeBaseContent.isEmpty())
    {
        return QString(); // Return empty string if not loaded
    }

    // Check if the query seems relevant to our knowledge base
    if (!containsRelevantInfo(query))
    {
        return QString(); // Return empty if not relevant
    }

    // Return the entire knowledge base content
    // The AI will extract relevant information from it
    return knowledgeBaseContent;
}

bool LocalKnowledgeInterface::containsRelevantInfo(const QString &query) const
{
    QString lowerQuery = query.toLower();

    // Keywords that suggest the query is about static information we might have
    QStringList localInfoKeywords = {
        "program", "programs", "degree", "degrees", "major", "majors",
        "mba", "bba", "business administration", "accounting", "finance",
        "marketing", "management", "accreditation", "aacsb",
        "location", "address", "contact", "phone", "email",
        "about", "what is", "tell me about", "information about"
    };

    // Check if query contains any of these keywords
    for (const QString& keyword : localInfoKeywords)
    {
        if (lowerQuery.contains(keyword))
        {
            return true;
        }
    }

    return false;
}
