import os, uuid, time, re, glob, json, requests
import chromadb
from chromadb.config import Settings

EMBED_MODEL = os.getenv("EMBED_MODEL", "nomic-embed-text")
OLLAMA_HOST = os.getenv("OLLAMA_HOST", "http://localhost:11434")
CHROMA_PATH = os.getenv("CHROMA_PATH", "./chroma_db")
COLLECTION = os.getenv("COLLECTION", "cob_docs")

CHUNK_SIZE = int(os.getenv("CHUNK_SIZE", "800"))
CHUNK_OVERLAP = int(os.getenv("CHUNK_OVERLAP", "120"))

os.environ["CHROMA_TELEMETRY_ENABLED"] = "false"

def _client_collection():
    client = chromadb.PersistentClient(path=CHROMA_PATH, settings=Settings(anonymized_telemetry=False))
    try:
        col = client.get_collection(COLLECTION)
    except Exception:
        col = client.create_collection(COLLECTION, metadata={"hnsw:space": "cosine"})
    return client, col

def chunk_text(text: str, size=CHUNK_SIZE, overlap=CHUNK_OVERLAP):
    n = len(text)
    if n <= size:
        return [text]
    chunks, start = [], 0
    while start < n:
        end = min(n, start + size)
        chunks.append(text[start:end])
        if end == n:
            break
        start = max(0, end - overlap)
    return chunks

def embed(text: str):
    r = requests.post(f"{OLLAMA_HOST}/api/embeddings",
                      json={"model": EMBED_MODEL, "prompt": text})
    r.raise_for_status()
    return r.json()["embedding"]

def upsert_text_doc(title: str, body: str, url: str = "", updated_at: str | None = None):
    if not updated_at:
        updated_at = time.strftime("%Y-%m-%d")
    client, collection = _client_collection()
    ids, docs, metas, vecs = [], [], [], []
    chunks = chunk_text(body)
    for i, chunk in enumerate(chunks):
        doc_id = f"{uuid.uuid4().hex[:8]}::{i}"
        vec = embed(chunk)
        ids.append(doc_id)
        docs.append(chunk)
        metas.append({"source": url or title, "title": title, "updated_at": updated_at})
        vecs.append(vec)
    if ids:
        collection.upsert(ids=ids, documents=docs, embeddings=vecs, metadatas=metas)
    return len(ids)

def reindex_pages_folder(folder="data/pages"):
    paths = sorted(glob.glob(os.path.join(folder, "*.txt")))
    total = 0
    for p in paths:
        raw = open(p, "r", encoding="utf-8").read()
        m_url = re.search(r"^URL:\s*(.+)$", raw, re.MULTILINE)
        m_upd = re.search(r"^UPDATED:\s*(.+)$", raw, re.MULTILINE)
        url = (m_url.group(1).strip() if m_url else "")
        updated_at = (m_upd.group(1).strip() if m_upd else time.strftime("%Y-%m-%d"))
        body = re.sub(r"^URL:.*\n?|^UPDATED:.*\n?", "", raw, flags=re.MULTILINE).strip()
        if not body:
            continue
        title = os.path.splitext(os.path.basename(p))[0].replace("_"," ").title()
        total += upsert_text_doc(title, body, url=url, updated_at=updated_at)
    return total

def retrieve(query: str, top_k: int = 4):
    _, collection = _client_collection()
    qvec = embed(query)
    n_docs = collection.count()
    k = min(top_k, n_docs) if n_docs else 0
    if k == 0:
        return []
    res = collection.query(query_embeddings=[qvec], n_results=k)
    docs = res.get("documents", [[]])[0]
    metas = res.get("metadatas", [[]])[0]
    seen, results = set(), []
    for doc, meta in zip(docs, metas):
        key = (meta.get("title"), meta.get("source"))
        if key in seen:
            continue
        seen.add(key)
        results.append((doc, meta))
    return results

def generate_with_ollama(model: str, prompt: str, temperature: float = 0.25, num_predict: int = 200):
    payload = {
        "model": model,
        "prompt": prompt,
        "options": {"temperature": temperature, "num_predict": num_predict},
        "stream": False
    }
    r = requests.post(f"{OLLAMA_HOST}/api/generate", json=payload, timeout=180)
    r.raise_for_status()
    try:
        data = r.json()
        if isinstance(data, dict) and "response" in data:
            return data["response"]
    except Exception:
        pass
    text = ""
    for line in r.iter_lines(decode_unicode=True):
        if not line:
            continue
        try:
            obj = json.loads(line)
            text += obj.get("response", "")
            if obj.get("done"):
                break
        except Exception:
            pass
    return text.strip()
