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
  <link rel="stylesheet" href="/main.css" >
</head>

<body>
  <main class="card">
    <h1>Radio station</h1>
    <p class="hint">Selecteer het radiostation en druk op <b>Selecteren</b>.</p>

    <form id="wifiForm" autocomplete="off">
      <div class="row">
        <section class="scroll-container">
            <!--OPTIONS-->
        </section>
      </div>

      <div class="actions">
        <button id="submitBtn" type="submit">Selecteren</button>
        <div id="status" class="status" aria-live="polite"></div>
      </div>
    </form>
  </main>

  <script>
    let observer = new IntersectionObserver(entries => {
      entries.forEach(entry => {
        with(entry) if(isIntersecting) target.children[1].checked = true;
      });
    }, {
      root: document.querySelector(`.scroll-container`), rootMargin: `-51% 0px -49% 0px`
    });

    document.querySelectorAll(`.scroll-item`).forEach(item => observer.observe(item));
  </script>

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
