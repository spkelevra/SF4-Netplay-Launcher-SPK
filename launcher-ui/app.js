(function () {
  const PROTOCOL_VERSION = 1;
  let state = {};

  function post(msg) {
    if (window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage(Object.assign({ v: PROTOCOL_VERSION }, msg));
    }
  }

  function showToast(text, kind) {
    const el = document.getElementById("toast");
    el.textContent = text;
    el.className = "toast " + (kind || "");
    el.classList.remove("hidden");
    clearTimeout(showToast._t);
    showToast._t = setTimeout(function () {
      el.classList.add("hidden");
    }, 4000);
  }

  function showScreen(id) {
    document.querySelectorAll(".screen").forEach(function (s) {
      s.classList.toggle("active", s.id === id);
    });
  }

  function applyState(s) {
    if (!s || s.type !== "state") return;
    state = s;
    const name = s.displayName || "Player";
    const delay = s.inputDelay || 2;
    const port = s.sessionPort || 23456;

    ["host-name", "join-name"].forEach(function (id) {
      const el = document.getElementById(id);
      if (el) el.value = name;
    });
    ["host-delay", "join-delay"].forEach(function (id) {
      const el = document.getElementById(id);
      if (el) el.value = delay;
    });
    const hostPort = document.getElementById("host-port");
    if (hostPort) hostPort.value = port;

    const lanCode = document.getElementById("host-lan-code");
    if (lanCode) lanCode.textContent = s.lanRoomCode || "—";

    const adv = document.getElementById("host-advertise");
    if (adv) adv.value = s.advertiseHost || "";

    const preview = document.getElementById("host-room-preview");
    if (preview) preview.textContent = s.roomCodePreview || "—";

    const joinAddr = document.getElementById("join-address");
    if (joinAddr && s.lastJoinHost) joinAddr.value = s.lastJoinHost;
  }

  function previewRoomCode() {
    const adv = document.getElementById("host-advertise");
    const port = document.getElementById("host-port");
    post({
      type: "previewRoomCode",
      advertiseHost: adv ? adv.value : "",
      sessionPort: port ? parseInt(port.value, 10) : 23456,
    });
  }

  if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener("message", function (ev) {
      let data = ev.data;
      if (typeof data === "string") {
        try {
          data = JSON.parse(data);
        } catch (e) {
          return;
        }
      }
      if (data.type === "state") {
        applyState(data);
      } else if (data.type === "error") {
        showToast(data.message || "Error", "error");
      } else if (data.type === "copied") {
        showToast("Copied to clipboard", "success");
      }
    });
  }

  document.querySelectorAll(".mode-card").forEach(function (btn) {
    btn.addEventListener("click", function () {
      const mode = btn.getAttribute("data-mode");
      if (mode === "host") showScreen("screen-host");
      else if (mode === "join") showScreen("screen-join");
      else if (mode === "offline") showScreen("screen-offline");
    });
  });

  document.querySelectorAll("[data-back]").forEach(function (btn) {
    btn.addEventListener("click", function () {
      showScreen("screen-home");
    });
  });

  document.getElementById("btn-refresh-ip").addEventListener("click", function () {
    post({ type: "fetchPublicIp" });
  });

  document.getElementById("host-advertise").addEventListener("input", previewRoomCode);
  document.getElementById("host-port").addEventListener("input", previewRoomCode);

  document.getElementById("btn-copy-room").addEventListener("click", function () {
    const text = document.getElementById("host-room-preview").textContent;
    post({ type: "copyText", text: text });
  });

  document.getElementById("btn-start-host").addEventListener("click", function () {
    post({
      type: "start",
      mode: "host",
      displayName: document.getElementById("host-name").value,
      inputDelay: parseInt(document.getElementById("host-delay").value, 10),
      sessionPort: parseInt(document.getElementById("host-port").value, 10),
      advertiseHost: document.getElementById("host-advertise").value,
    });
  });

  document.getElementById("btn-start-join").addEventListener("click", function () {
    post({
      type: "start",
      mode: "join",
      connectMethod: "direct",
      displayName: document.getElementById("join-name").value,
      inputDelay: parseInt(document.getElementById("join-delay").value, 10),
      joinAddress: document.getElementById("join-address").value,
    });
  });

  document.getElementById("btn-start-offline").addEventListener("click", function () {
    post({ type: "start", mode: "offline" });
  });

  post({ type: "getState" });
})();
