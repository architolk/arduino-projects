#ifndef _EVENTS_PAGE_H
#define _EVENTS_PAGE_H

const char EVENTS_HTML[] PROGMEM = R"=====(
<html>
<body>
  <h2>ESP32 SSE Demo</h2>
  <div id="log"></div>

  <script>
    const log = document.getElementById("log");
    const source = new EventSource("/api/events");

    source.onmessage = (event) => {
      console.log(event.data);
      log.innerHTML += event.data + "<br>";
    };
  </script>
</body>
</html>

)=====";

#endif
