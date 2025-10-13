import os, pathlib, datetime, re, requests
from fastapi import FastAPI, Request, Form
from fastapi.responses import HTMLResponse, JSONResponse, RedirectResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from bs4 import BeautifulSoup

from .rag_core import reindex_pages_folder, upsert_text_doc, retrieve, generate_with_ollama
from .rag_core import EMBED_MODEL, CHROMA_PATH, COLLECTION
from .hours_logic import is_open_now_from_text

APP_DIR = pathlib.Path(__file__).parent
ROOT_DIR = APP_DIR.parent
DATA_DIR = ROOT_DIR / "data" / "pages"

LLM_MODEL = os.getenv("LLM_MODEL", "gemma3:1b")
TEMPERATURE = float(os.getenv("TEMPERATURE", "0.25"))
MAX_NEW_TOKENS = int(os.getenv("MAX_NEW_TOKENS", "200"))

app = FastAPI(title="CoB Kiosk RAG Server")
app.mount("/static", StaticFiles(directory=str(APP_DIR / "static")), name="static")
templates = Jinja2Templates(directory=str(APP_DIR / "templates"))

@app.get("/", response_class=HTMLResponse)
async def home(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})

@app.get("/admin", response_class=HTMLResponse)
async def admin(request: Request):
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    files = sorted([p.name for p in DATA_DIR.glob("*.txt")])
    return templates.TemplateResponse("admin.html", {"request": request, "files": files})

@app.post("/admin/add_text")
async def admin_add_text(title: str = Form(...), url: str = Form(""), body: str = Form(...)):
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    slug = re.sub(r"[^a-z0-9]+", "_", title.lower()).strip("_")
    path = DATA_DIR / f"{slug}.txt"
    path.write_text(f"URL: {url}\nUPDATED: {datetime.date.today()}\n\n{body}", encoding="utf-8")
    upsert_text_doc(title, body, url=url, updated_at=str(datetime.date.today()))
    return RedirectResponse(url="/admin", status_code=303)

@app.post("/admin/fetch_url")
async def admin_fetch_url(url: str = Form(...)):
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    r = requests.get(url, timeout=20)
    r.raise_for_status()
    soup = BeautifulSoup(r.text, "html.parser")
    for tag in soup(["script","style","nav","footer","header"]):
        tag.decompose()
    text = soup.get_text("\n")
    text = re.sub(r"\n\s*\n+", "\n\n", text).strip()
    title = soup.title.string.strip() if soup.title and soup.title.string else url
    slug = re.sub(r"[^a-z0-9]+", "_", title.lower()).strip("_")
    path = DATA_DIR / f"{slug}.txt"
    path.write_text(f"URL: {url}\nUPDATED: {datetime.date.today()}\n\n{text}", encoding="utf-8")
    upsert_text_doc(title, text, url=url, updated_at=str(datetime.date.today()))
    return RedirectResponse(url="/admin", status_code=303)

@app.post("/api/ingest")
async def api_ingest():
    total = reindex_pages_folder(str(DATA_DIR))
    return JSONResponse({"ok": True, "chunks_indexed": total})

@app.post("/api/ask")
async def api_ask(payload: dict):
    q = payload.get("query","").strip()
    if not q:
        return JSONResponse({"ok": False, "error": "empty query"}, status_code=400)

    ctx = retrieve(q, top_k=4)

    qlow = q.lower()
    if any(k in qlow for k in ["open", "closed", "today", "now", "hours today"]) and ctx:
        hours_text = "\n\n".join(doc for doc, _ in ctx)[:8000]
        is_open, msg, range_human = is_open_now_from_text(hours_text)
        if is_open is not None:
            meta0 = ctx[0][1]
            citation = {
                "title": meta0.get("title","Source"),
                "source": meta0.get("source",""),
                "updated_at": meta0.get("updated_at","")
            }
            answer = f"{'Yes' if is_open else 'No'}. {msg}"
            if range_human:
                answer += f" Regular hours are {range_human}."
            return JSONResponse({"ok": True, "answer": answer, "sources":[citation], "fastpath":"hours"})

    ctx_lines = []
    for i, (doc, meta) in enumerate(ctx, 1):
        ctx_lines.append(f"Source {i} ({meta.get('title','Source')}, updated {meta.get('updated_at','')}, source {meta.get('source','')}):\n{doc}")
    context = "\n\n".join(ctx_lines) if ctx_lines else "(no context found)"
    today = datetime.datetime.now().strftime("%A")
    prompt = (
        "You are Zippy AI, the College of Business kiosk. "
        "Answer ONLY using the provided context snippets. "
        "If information is missing or confidence is low, say you don't know and refer to the front desk. "
        "Prefer short, clear answers.\n\n"
        f"Today is {today}.\n"
        f"CONTEXT:\n{context}\n\n"
        f"QUESTION:\n{q}\n\n"
        "Answer concisely using ONLY the context above. If missing, say you don't know."
    )
    text = generate_with_ollama(LLM_MODEL, prompt)
    sources = [{"title": m.get("title","Source"), "source": m.get("source",""), "updated_at": m.get("updated_at","")} for _, m in ctx]
    return JSONResponse({"ok": True, "answer": text, "sources": sources, "fastpath": None})

@app.get("/api/status")
async def api_status():
    return {"ok": True, "embed_model": EMBED_MODEL, "collection": COLLECTION, "chroma_path": CHROMA_PATH}
