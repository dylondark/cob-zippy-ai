#ifndef LOCALKNOWLEDGEINTERFACE_H
#define LOCALKNOWLEDGEINTERFACE_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QtQmlIntegration>

/*
    Interface for searching local knowledge base files
*/
class LocalKnowledgeInterface : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LocalKnowledgeInterface(QObject *parent = nullptr);

    // Search the knowledge base for relevant information
    QString search(const QString &query);

    // Load knowledge base from file
    bool loadKnowledgeBase(const QString &filePath);

    // Check if knowledge base is loaded
    bool isLoaded() const;

private:
    QString knowledgeBaseContent;
    bool loaded;

    // Simple keyword-based search
    bool containsRelevantInfo(const QString &query) const;
};

#endif // LOCALKNOWLEDGEINTERFACE_H
