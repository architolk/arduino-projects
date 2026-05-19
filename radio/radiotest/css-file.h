#ifndef _CSS_FILE_H
#define _CSS_FILE_H

const char MAIN_CSS[] PROGMEM = R"=====(
:root {
  --bg: #0b1220;
  --card: #111a2e;
  --text: #eaf0ff;
  --muted: #a9b6d6;
  --border: rgba(255,255,255,.12);
  --primary: #3b82f6;
  --primaryHover: #2563eb;
  --danger: #ef4444;
  --ok: #22c55e;
  --disabled: #94a3b8;
}

* { box-sizing: border-box; }
body {
  margin: 0;
  font-family: system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, sans-serif;
  background: radial-gradient(1000px 600px at 20% 0%, #14214a 0%, var(--bg) 55%);
  color: var(--text);
  min-height: 100svh;
  display: grid;
  place-items: center;
  padding: 20px;
}

.card {
  width: min(520px, 100%);
  background: color-mix(in srgb, var(--card) 92%, transparent);
  border: 1px solid var(--border);
  border-radius: 16px;
  padding: 18px;
  box-shadow: 0 12px 40px rgba(0,0,0,.35);
}

h1 {
  font-size: 1.15rem;
  margin: 0 0 12px 0;
  letter-spacing: .2px;
}

.hint {
  margin: 0 0 16px 0;
  color: var(--muted);
  font-size: .95rem;
  line-height: 1.35;
}

label {
  display: block;
  margin: 12px 0 6px;
  font-size: .95rem;
  color: var(--muted);
}

input, textarea {
  width: 100%;
  padding: 12px 12px;
  border-radius: 12px;
  border: 1px solid var(--border);
  background: rgba(255,255,255,.03);
  color: var(--text);
  outline: none;
  font-size: 1rem;
}
input:focus, textarea:focus {
  border-color: color-mix(in srgb, var(--primary) 60%, white);
  box-shadow: 0 0 0 4px rgba(59,130,246,.18);
}

.row {
  display: grid;
  gap: 12px;
}

.column3 {
  display: grid;
  grid-template-columns:  1fr 1fr 1fr;
  gap: 12px;
}

.actions {
  display: grid;
  gap: 10px;
  margin-top: 16px;
}

button {
  width: 100%;
  padding: 12px 14px;
  border-radius: 12px;
  border: 0;
  background: var(--primary);
  color: white;
  font-weight: 650;
  font-size: 1rem;
  cursor: pointer;
  transition: transform .02s ease, background .15s ease, opacity .15s ease;
}
button:hover { background: var(--primaryHover); }
button:active { transform: translateY(1px); }

button[disabled] {
  background: var(--disabled);
  cursor: not-allowed;
  opacity: .85;
}

.status {
  min-height: 1.2em;
  font-size: .95rem;
  color: var(--muted);
}
.status.ok { color: var(--ok); }
.status.err { color: var(--danger); }

/* Klein beetje extra comfort op mobiel */
@media (max-width: 420px) {
  .card { padding: 16px; border-radius: 14px; }
  input, button { border-radius: 12px; }
}
)=====";

#endif
