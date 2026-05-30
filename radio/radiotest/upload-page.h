#ifndef _UPLOAD_PAGE_H
#define _UPLOAD_PAGE_H

const char UPLOAD_HTML[] PROGMEM = R"=====(
<!doctype html>
<html lang="nl">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Radio - Upload file</title>
  <link rel="stylesheet" href="/main.css" >
</head>

<body>
  <main class="card">
    <h1>File upload</h1>
    <p class="hint">Geef bestand met radio stations</p>

    <form id="uploadform" method="POST" enctype="multipart/form-data">
      <div class="row">
        <div>
          <label for="file">Radio stations file</label>
          <input id="file" name="file" type="file" required />
        </div>

      </div>

      <div class="actions">
        <div class="column3">
          <button id="submitBtn" type="submit">Upload</button>
        </div>
      </div>
    </form>
    <div class="row">
      <div class="actions">
        <div class="column3">
          <button id="downloadBtn" onClick="downloadFile();">Download</button>
          <button id="deleteBtn" onClick="deleteFile();">Delete</button>
        </div>
        <div id="status" class="status" aria-live="polite"></div>
      </div>
    </div>
  </main>

<script>
const API_URL = "/api/upload";
const DELETE_API_URL = "/api/deletefile";

const statusEl = document.getElementById("status");
const btn = document.getElementById("submitBtn");

function setStatus(msg, type) {
  statusEl.textContent = msg || "";
  statusEl.classList.remove("ok", "err");
  if (type) statusEl.classList.add(type);
}

var form = document.getElementById('uploadform');
form.onsubmit = async (e) => {
  e.preventDefault();
  setStatus("Bezig...", null);
  btn.disabled = true;
  const form = e.currentTarget;

  try {
    const formData = new FormData(form);
    const resp = await fetch(API_URL, {
      method: 'POST',
      body: formData
    });

    btn.disabled = false;
    if (resp.ok) { // 200-299
      setStatus("Bestand opgeslagen", "ok");
    } else {
      // Probeer een foutmelding uit de response te halen (als die er is)
      let details = "";
      try { details = await resp.text(); } catch (_) {}
      setStatus(`Fout: server antwoordde met ${resp.status}${details ? " – " + details : ""}`, "err");
    }
  } catch (err) {
    btn.disabled = false;
    setStatus("Netwerkfout: kon de API niet bereiken.", "err");
  }

}

function downloadFile() {
  window.open("stations.txt");
}

async function deleteFile() {
  setStatus("Bezig met verwijderen...", null);
  const action = 0;
  try {
    const resp = await fetch(DELETE_API_URL, {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify({ action})
    });

    if (resp.ok) { // 200-299
      setStatus("Bestand verwijderd", "ok");
    } else {
      // Probeer een foutmelding uit de response te halen (als die er is)
      let details = "";
      try { details = await resp.text(); } catch (_) {}
      setStatus(`Fout: server antwoordde met ${resp.status}${details ? " – " + details : ""}`, "err");
    }
  } catch (err) {
    setStatus("Netwerkfout: kon de API niet bereiken.", "err");
  }
}
</script>

</body>
</html>

)=====";

#endif
