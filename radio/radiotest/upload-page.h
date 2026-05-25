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
    <p class="hint">Geef file met stations</p>

    <form id="uploadform" method="POST" enctype="multipart/form-data">
      <div class="row">
        <div>
          <label for="file">SSID</label>
          <input id="file" name="file" inputmode="file" required />
        </div>

      </div>

      <div class="actions">
        <button id="submitBtn" type="submit">Instellen</button>
        <div id="status" class="status" aria-live="polite"></div>
      </div>
    </form>
  </main>

<script>
const API_URL = "/api/upload";

var form = document.getElementById('uploadform');

form.onsubmit = async (e) => {
  e.preventDefault();
  const form = e.currentTarget;

  try {
      const formData = new FormData(form);
      const response = await fetch(API_URL, {
          method: 'POST',
          body: formData
      });

      console.log(response);
  } catch (error) {
      console.error(error);
  }

}
</script>

</body>
</html>

)=====";

#endif
