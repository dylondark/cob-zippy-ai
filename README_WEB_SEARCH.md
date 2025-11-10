# Setting Up Ollama Web Search

This application uses Ollama's official Web Search API to provide real-time information.

## Getting Your API Key

1. Visit [https://ollama.com](https://ollama.com)
2. Create a free Ollama account (if you don't have one)
3. Go to your account settings and create an API key
4. Copy your API key

## Configuring the API Key

1. Open the file `ollama_api_key.txt` in the project directory
2. Replace `YOUR_OLLAMA_API_KEY_HERE` with your actual API key
3. Save the file

Example:
```
ol-1234567890abcdefghijklmnopqrstuv
```

## How It Works

The application will:
1. **First**: Check the local knowledge base (knowledge_base.txt) for static information
2. **Second**: If not found and the query needs current info, use Ollama's Web Search API
3. **Third**: Fall back to the AI's general knowledge

## Web Search Features

- Returns up to 5 relevant search results
- Includes title, content snippets, and URLs from each result
- Automatically includes current date/time context
- Properly formats results for the AI to process

## Troubleshooting

**Error: "Ollama API key not configured"**
- Make sure you created the `ollama_api_key.txt` file
- Make sure it contains your actual API key (not the placeholder text)
- Make sure the file is in the same directory as the executable

**Error: "Web search authentication failed"**
- Your API key might be invalid or expired
- Double-check that you copied the entire API key correctly
- Try generating a new API key from your Ollama account

## Cost

Ollama offers a free tier for their web search API. Check [https://ollama.com/pricing](https://ollama.com/pricing) for current limits and pricing.
