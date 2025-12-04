# Zippy AI - College of Business Assistant

A desktop AI chatbot application designed for the University of
Akron College of Business. Zippy provides students and faculty with
an intelligent assistant to answer questions about the college,
programs, and general inquiries.

![Zippy AI
Screenshot](https://github.com/user-attachments/assets/3ad1debf-a664-46f6-9cde-0afb524b9aa7)

## Features

- Real-time chat interface with AI-powered responses
- Local AI model integration via Ollama
- Conversation history management
- Clean, modern Qt-based UI
- Cross-platform support (Windows, macOS, GNU/Linux)

## Requirements

- **Qt 6.8 or higher** - For building the application
- **Ollama** - For running the AI models locally
- **qwen3:4b model** (default) - Downloaded automatically by Ollama

## Getting Started

### 1. Install Ollama

Download and install Ollama from: **https://ollama.com**

After installation, Ollama will run in the background.

### 2. Download the AI Model

Open a terminal/command prompt and run:

```bash
ollama pull qwen3:4b
```

This downloads the default AI model used by Zippy (approximately
2.3GB).

### 3. Run the Application

**Important:** Ollama must be running before you start Zippy AI.
Ollama typically runs automatically in the background after
installation.

Then simply launch `appcob_zippy_ai.exe` (Windows) or the compiled
executable for your platform.

## Building from Source

### Prerequisites

- Qt 6.8 or higher
- CMake 3.16 or higher
- C++17 compatible compiler
- Ollama installed and running

### Build Steps

```bash
# Clone the repository
git clone https://github.com/dylondark/cob-zippy-ai.git
cd cob-zippy-ai

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# Run
./appcob_zippy_ai
```

## Configuration

You can configure Zippy through the settings panel:

- **Ollama URL**: Default is `http://localhost:11434`
- **Model**: Default is `qwen3:4b` (you can change to other Ollama
  models)
- **Context Size**: Adjustable for longer conversations
- **Timeout**: Request timeout in seconds

## Available Models

While Zippy uses `qwen3:4b` by default, you can use any model
available through Ollama:

```bash
# List available models
ollama list

# Pull a different model
ollama pull llama2
ollama pull mistral
```

Then change the model in Zippy's settings.

**Please note that Zippy AI requires Ollama to be running in the
background.**  The AI responses are generated locally on your machine
using the Ollama framework.  Make sure that both Ollama and the
model you wish to use with it are both installed prior to using the
AI.

## Troubleshooting

### "Not connected to Ollama server"

- Ensure Ollama is installed and running
- Check if Ollama is accessible at `http://localhost:11434`
- Try running `ollama list` in terminal to verify Ollama is working

### Model not found

- Make sure you've pulled the model: `ollama pull qwen3:4b`
- Check available models: `ollama list`

### Application won't start

- Verify Qt 6.8+ is installed
- Check that all dependencies are present
- On Windows, you may need Visual C++ Redistributable

## Contributing

Contributions are welcome! Please feel free to submit issues or
pull requests.

## License

Zippy AI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Credits

Developed for the [University of Akron College of
Business](https://www.uakron.edu/cba/)\
Powered by [Ollama](https://ollama.com) and [Qt
Framework](https://www.qt.io/development/qt-framework)
