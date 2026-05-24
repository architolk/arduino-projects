#ifndef _CONFIG_PAGE_H
#define _CONFIG_PAGE_H

const char CONFIG_HTML[] PROGMEM = R"=====(
<!doctype html>
<html lang="nl">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Radio - WiFi instellen</title>
  <link rel="stylesheet" href="/main.css" >
</head>

<body>
  <main class="card">
    <h1>WiFi configuratie</h1>
    <p class="hint">Vul SSID en wachtwoord in en druk op <b>Instellen</b>. Bij succes wordt dit <b>Geregistreerd</b>.</p>

    <form id="wifiForm" autocomplete="off">
      <div class="row">
        <div>
          <label for="ssid">SSID</label>
          <input id="ssid" name="ssid" inputmode="text" placeholder="Bijv. MijnNetwerk" list="ssidlist" required />
          <datalist id="ssidlist">
            %OPTIONS%
          </datalist>
        </div>

        <div>
          <label for="password">Password</label>
          <input id="password" name="password" type="password" placeholder="••••••••" required />
        </div>
      </div>

      <div class="actions">
        <button id="submitBtn" type="submit">Instellen</button>
        <div id="status" class="status" aria-live="polite"></div>
      </div>
    </form>
  </main>

  <script>
    const API_URL = "/api/wificonfig";

    const form = document.getElementById("wifiForm");
    const ssidEl = document.getElementById("ssid");
    const passEl = document.getElementById("password");
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
      btn.textContent = "Instellen";
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
    passEl.addEventListener("input", onInputChanged);

    form.addEventListener("submit", async (e) => {
      e.preventDefault();
      if (busy) return;

      const ssid = ssidEl.value.trim();
      const password = passEl.value;

      if (!ssid || !password) {
        setStatus("Vul SSID en wachtwoord in.", "err");
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
