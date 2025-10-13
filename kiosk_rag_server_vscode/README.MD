# CoB Kiosk RAG (Local + Ollama)

Local, private “Ask the College of Business” kiosk.

- **UI:** FastAPI + HTML templates  
- **Search:** Chroma (local vector DB) + `nomic-embed-text` (Ollama)  
- **LLM:** default `gemma3:1b` via Ollama  
- **One-click run:** open `run.py` in PyCharm/VS Code and click **Run ▶️**

---

## Quick Start

### 1) Prerequisites (one time per machine)
- **Python 3.11 (64-bit)**  
- **Ollama** installed & running: <https://ollama.com>  
- Pull models once:
  ```powershell
  ollama pull nomic-embed-text
  ollama pull gemma3:1b
