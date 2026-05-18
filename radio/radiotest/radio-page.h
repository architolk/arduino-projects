#ifndef _RADIO_PAGE_H
#define _RADIO_PAGE_H

// Veranderen met:
// https://css-tricks.com/how-to-make-a-scroll-to-select-form-control/

const char RADIO_HTML[] PROGMEM = R"=====(
<!doctype html>
<html lang="nl">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Radio - select station</title>
  <style>
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

    input {
      width: 100%;
      padding: 12px 12px;
      border-radius: 12px;
      border: 1px solid var(--border);
      background: rgba(255,255,255,.03);
      color: var(--text);
      outline: none;
      font-size: 1rem;
    }
    input:focus {
      border-color: color-mix(in srgb, var(--primary) 60%, white);
      box-shadow: 0 0 0 4px rgba(59,130,246,.18);
    }

    .row {
      display: grid;
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
  </style>
</head>

<body>
  <main class="card">
    <h1>Radio station</h1>
    <p class="hint">Selecteer het radiostation en druk op <b>Selecteren</b>.</p>

    <form id="wifiForm" autocomplete="off">
      <div class="row">
        <div>
          <label for="station">Station</label>
          <input id="station" name="station" inputmode="text" placeholder="Bijv. MijnNetwerk" list="stationlist" required />
          <datalist id="stationlist">
            <!--OPTIONS-->
          </datalist>
        </div>

      </div>

      <div class="actions">
        <button id="submitBtn" type="submit">Selecteren</button>
        <div id="status" class="status" aria-live="polite"></div>
      </div>
    </form>
  </main>

  <script>
    const API_URL = "/api/setstation";

    const form = document.getElementById("wifiForm");
    const stationEl = document.getElementById("station");
    const btn = document.getElementById("submitBtn");
    const statusEl = document.getElementById("status");

    let registered = false;
    let busy = false;

    function setStatus(msg, type) {
      statusEl.textContent = msg || "";
      statusEl.classList.remove("ok", "err");
      if (type) statusEl.classList.add(type);
    }

    function setButton(state) {
      // state: "ready" | "busy" | "registered"
      if (state === "registered") {
        btn.textContent = "Geregistreerd";
        btn.disabled = true;
        registered = true;
        busy = false;
        return;
      }
      if (state === "busy") {
        btn.textContent = "Bezig…";
        btn.disabled = true;
        busy = true;
        registered = false;
        return;
      }
      // ready
      btn.textContent = "Selecteren";
      btn.disabled = false;
      busy = false;
      registered = false;
    }

    // Als gebruiker iets wijzigt: knop terug naar "Instellen"
    function onInputChanged() {
      if (registered || btn.disabled) {
        setButton("ready");
        setStatus("", null);
      }
    }
    ssidEl.addEventListener("input", onInputChanged);

    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      if (busy) return;

      const station = stationEl.value.trim();

      if (!station) {
        setStatus("Vul een geldig station in.", "err");
        return;
      }

      setButton("busy");
      setStatus("Instellen…", null);

      try {
        const resp = await fetch(API_URL, {
          method: "POST",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify({ ssid, password })
        });

        if (resp.ok) { // 200-299
          setButton("registered");
          setStatus("Succesvol geregistreerd.", "ok");
        } else {
          // Probeer een foutmelding uit de response te halen (als die er is)
          let details = "";
          try { details = await resp.text(); } catch (_) {}
          setButton("ready");
          setStatus(`Fout: server antwoordde met ${resp.status}${details ? " – " + details : ""}`, "err");
        }
      } catch (err) {
        setButton("ready");
        setStatus("Netwerkfout: kon de API niet bereiken.", "err");
      }
    });
  </script>
</body>
</html>

)=====";

#endif
