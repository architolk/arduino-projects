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
    <p class="hint">Selecteer het radiostation en kies de actie. Resultaten blijven bewaard tot de radio uit wordt gezet. Kies <save> om definitief te bewaren.</p>

    <div class="row">
      <section id="stationSection" class="scroll-container" onScroll="scrolling();">
          %OPTIONS%
      </section>
    </div>

    <div class="actions">
      <div class="column4">
        <button id="editBtn" onClick="editStation();">Edit</button>
        <button id="addBtn" onClick="addStation();">Add</button>
        <button id="delBtn" onClick="delStation();">Delete</button>
        <button id="saveBtn" onClick="saveStations();" disabled>Save</button>
      </div>
      <div id="status" class="status" aria-live="polite"></div>
    </div>
  </main>

  <script>
  const API_URL = "/api/stations";
  const SAVE_API_URL = "/api/savestations";
  const statusEl = document.getElementById("status");
  const saveBtn = document.getElementById("saveBtn");

  let observer = new IntersectionObserver(entries => {
    entries.forEach(entry => {
      with(entry) if(isIntersecting) target.children[1].checked = true;
    });
  }, {
    root: document.querySelector(`.scroll-container`), rootMargin: `-51%% 0px -49%% 0px`
  });

  const list = document.getElementById("stationSection");
  const inputURL = document.createElement("input");
  let editedLabel = null;
  inputURL.type="text";
  inputURL.style.visible="hidden";
  inputURL.style.position = "absolute";
  inputURL.style.width="0px";
  inputURL.style.top="-100px"; //Really hide the input
  const inputFreq = document.createElement("input");
  inputFreq.type="text";
  inputFreq.style.visible="hidden";
  inputFreq.style.position = "absolute";
  inputFreq.style.width = "65px";
  inputFreq.style.top="-100px"; //Really hide the input
  inputFreq.addEventListener("keypress", inputKeyPress);
  inputURL.addEventListener("keypress", inputKeyPress);
  list.appendChild(inputFreq);
  list.appendChild(inputURL);

  document.querySelectorAll(`.scroll-item`).forEach(item => observer.observe(item));

  function inputKeyPress(event) {
    if (event.keyCode==13) {
      scrolling();
    }
    if (event.keyCode==27) {
      scrollingIgnore();
    }
  }

  function getFrequency(text) {
    const freq = ~~(10*parseFloat(text));
    if ((freq<875) || (freq>1080)) {
      return 0
    } else {
      return freq;
    }
  }

  function scrolling() {
    if (editedLabel) {
      if (inputURL.value!="") {
        const newFreq = getFrequency(inputFreq.value);
        if (newFreq>0) {
          editedLabel.childNodes[0].textContent = inputURL.value;
          const span = editedLabel.querySelector("span");
          const freq = getFrequency(span.textContent);
          span.textContent = inputFreq.value;
          if (freq>0) {
            sendToServer(3,freq,newFreq,inputURL.value); //Update
          } else {
            sendToServer(2,newFreq,newFreq,inputURL.value); //Add
          }
        }
      }
      scrollingIgnore();
    }
  }

  function scrollingIgnore() {
    if (editedLabel) {
      inputURL.style.visibility="hidden";
      inputFreq.style.visibility="hidden";
      const span = editedLabel.querySelector("span");
      if (span.textContent=="") {
        //If the span doesn't have a frequency, it should be deleted
        editedLabel.remove();
      } else {
        editedLabel.style.visibility="visible";
      }
      editedLabel=null;
    }
  }

  function editStation() {
    const selectedRadio = document.querySelector('input[name="stations"]:checked');

    if (!selectedRadio) return;
    editedLabel = selectedRadio.parentElement;
    updateStation();
  }

  function updateStation() {

    if (!editedLabel) return;

    const span = editedLabel.querySelector("span");
    if (!span) return;

    inputURL.value = editedLabel.childNodes[0].textContent.trim();
    inputFreq.value = span.textContent;

    const rect = editedLabel.getBoundingClientRect();
    const parentRect = editedLabel.offsetParent.getBoundingClientRect();

    inputURL.style.left = `${70+rect.left - parentRect.left}px`;
    inputURL.style.top = `${8+rect.top - parentRect.top}px`;
    inputURL.style.width = `${rect.width-70}px`;
    inputFreq.style.left = `${rect.left - parentRect.left}px`;
    inputFreq.style.top = `${8+rect.top - parentRect.top}px`;

    editedLabel.style.visibility = "hidden";
    inputURL.style.visibility = "visible";
    inputFreq.style.visibility = "visible";
    inputFreq.select();
  }

  function addStation() {
    const list = document.getElementById("stationSection");
    const label = document.createElement("label");
    label.textContent = "https://radio-station-url/";
    label.className = "scroll-item";
    const span = document.createElement("span");
    //span.textContent = "100.0";
    label.appendChild(span);
    const radio = document.createElement("input");
    radio.type = "radio";
    radio.name = "stations";
    label.appendChild(radio);
    list.appendChild(label);
    observer.observe(label);
    list.scrollTop = list.scrollHeight; //Scroll to the end, set focus on the new element

    requestAnimationFrame(() => {
      editedLabel = label;
      updateStation();
    });
  }

  function delStation() {
    const selectedRadio = document.querySelector('input[name="stations"]:checked');
    const label = selectedRadio.parentElement;

    if (!label) return;

    const span = label.querySelector("span");
    if (!span) return;

    const freq = getFrequency(span.textContent);
    if (freq==0) return;

    label.remove();
    sendToServer(1,freq,freq,"-");
  }

  //
  // Handling interaction with server
  //

  function setStatus(msg, type) {
    statusEl.textContent = msg || "";
    statusEl.classList.remove("ok", "err");
    if (type) statusEl.classList.add(type);
  }

  async function sendToServer(action,freq,newfreq,url) {
    setStatus("Bezig met instellen...", null);
    try {
      const resp = await fetch(API_URL, {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({ action, freq, newfreq, url })
      });

      if (resp.ok) { // 200-299
        setStatus("Aangepast", "ok");
        saveBtn.disabled = false;
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

  async function saveStations() {
    setStatus("Bezig met opslaan...", null);
    saveBtn.disabled = true;
    const action = 0;
    try {
      const resp = await fetch(SAVE_API_URL, {
        method: "POST",
        headers: {
          "Content-Type": "application/json"
        },
        body: JSON.stringify({ action})
      });

      if (resp.ok) { // 200-299
        setStatus("Radiostations opgeslagen", "ok");
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
