#ifndef _EVENTS_PAGE_H
#define _EVENTS_PAGE_H

const char EVENTS_HTML[] PROGMEM = R"=====(
<html lang="nl">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Radio - Info</title>
  <link rel="stylesheet" href="/main.css" >
</head>

<body>
  <main class="card">
    <h1>Radio Marco</h1>

    <div class="row">
      <div>
        <label for="station">Radio station</label>
        <input id="station" name="station" value="%STATIONNAME%" readonly />
      </div>

      <div class="column3">
        <div>
          <label for="volume">Volume</label>
          <input id="volume" name="volume" value="" readonly />
        </div>
        <div>
          <label for="treble">Treble</label>
          <input id="treble" name="treble" value="" readonly />
        </div>
        <div>
          <label for="bass">Bass</label>
          <input id="bass" name="bass" value="" readonly />
        </div>
      </div>

      <div>
        <label for="song">Song</label>
        <input id="song" name="song" value="%SONGTITLE%" readonly />
      </div>

      <div>
        <label for="info">Informatie</label>
        <textarea id="info" name="info" rows="10" cols="150" readonly ></textarea>
      </div>

    </div>
  </main>

  <script>
    const source = new EventSource("/api/events");
    source.addEventListener("info", (event) => {
      const info = document.getElementById("info");
      info.innerHTML += event.data + "\n";
    });
    source.addEventListener("volume", (event) => {
      const elem = document.getElementById("volume");
      elem.value = event.data;
    });
    source.addEventListener("treble", (event) => {
      const elem = document.getElementById("treble");
      elem.value = event.data;
    });
    source.addEventListener("bass", (event) => {
      const elem = document.getElementById("bass");
      elem.value = event.data;
    });
    source.addEventListener("song", (event) => {
      const elem = document.getElementById("song");
      elem.value = event.data;
    });
    source.addEventListener("station", (event) => {
      const elem = document.getElementById("station");
      elem.value = event.data;
    });
  </script>
</body>
</html>
)=====";

#endif
