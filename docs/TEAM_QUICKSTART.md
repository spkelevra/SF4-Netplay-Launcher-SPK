# SF4 Enhanced — team test package

This folder is a **self-contained SF4 Enhanced build** (sf4e fork) for netplay testing. It does not include the game itself.

Upstream: [sf4e by adanducci](https://codeberg.org/adanducci/sf4e). See `ATTRIBUTION.md` in the full repository.

## Quick start (3 steps)

1. **Extract** the entire zip to one folder (e.g. `C:\Games\SF4-Enhanced\`). Do not copy only `Launcher.exe`.
2. **Install once** (if you have not already):
   - [Microsoft Edge WebView2 Runtime](https://go.microsoft.com/fwlink/p/?LinkId=2124703)
   - [VC++ Redistributable (x86)](https://aka.ms/vs/17/release/vc_redist.x86.exe)
   - **Ultra Street Fighter IV** on Steam (app ID 45760)
3. **Run** `preflight.ps1` (optional sanity check), then **`Launcher.exe`** from that folder → **Host**, **Join**, or **Offline**.

```powershell
powershell -ExecutionPolicy Bypass -File preflight.ps1
```

## What you need for netplay

- Two PCs on the **same LAN**, or the host **port-forwards** session port **23456** (TCP/UDP).
- **Same zip on both players** — do not mix `Sidecar.dll` from another build.

## File checklist

Confirm these sit **next to each other** in one folder:

- `Launcher.exe`, `Sidecar.dll`, `WebView2Loader.dll`
- `launcher-ui\` (`index.html`, `app.js`, `styles.css`)
- `spdlog.dll`, `fmt.dll`, `GameNetworkingSockets.dll`, `GGPO.dll`, `libcrypto-3.dll`, `libprotobuf.dll`, `abseil_dll.dll`

See `MANIFEST.txt` in the package to verify extraction.

## Run the launcher

1. Double-click **`Launcher.exe`** (or `Launcher.exe --console` for logs).
2. In the WebView2 launcher:
   - **Host** — set name and delay; **Copy** the room code (`IP:port`) and send it to your opponent.
   - **Join** — paste the host’s room code into **Host address** and enter your name.
   - **Offline** — game only, no netplay session.
3. Click **Start game**. sf4e should find USF4 via Steam automatically.

If Steam detection fails:

```bat
set STEAM_APP_PATH=C:\Program Files (x86)\Steam\steamapps\common\Super Street Fighter IV - Arcade Edition
Launcher.exe
```

## Firewall

- **Host:** allow inbound **TCP/UDP** on the session port (default **23456**).
- Relay mode is on by default — you usually **do not** need separate GGPO UDP port forwards.

## Quick 2-player test

| Step | Host | Joiner |
|------|------|--------|
| 1 | Launcher → **Host** → Start game | Launcher → **Join** → paste room code → Start game |
| 2 | Wait in lobby | Connect |
| 3 | Both **Ready** in-game | Both **Ready** |
| 4 | Play a few rounds | Same zip / `Sidecar.dll` |

Full checklist: [SMOKE_TEST.md](SMOKE_TEST.md). Player guide: [USER_NETPLAY.md](USER_NETPLAY.md).

## Troubleshooting

| Problem | What to try |
|---------|-------------|
| `WebView2Loader.dll was not found` | Re-extract the **full** zip; keep all files beside `Launcher.exe` |
| Launcher says WebView2 required | Install [WebView2 Runtime](https://go.microsoft.com/fwlink/p/?LinkId=2124703) |
| Blank launcher window | Ensure **`launcher-ui\`** is next to `Launcher.exe` |
| Double-click does nothing | Run `Launcher.exe --console` from a terminal in the package folder |
| “Version mismatch” on join | Same zip on both PCs |
| Join times out | Same LAN, correct room code, host firewall, host port open |
| Missing other DLL errors | Re-extract full zip; install [VC++ x86](https://aka.ms/vs/17/release/vc_redist.x86.exe) |
| Host/Join issues at menu | Open in-game **Network** panel; both **Ready** |
| Need logs | `Launcher.exe --console` or `%APPDATA%\sf4e\` |

Report issues with the **Git** line from `BUILD_INFO.txt` and a screenshot.

## Developer overlay (optional)

```bat
set SF4E_NETPLAY_DEV=1
Launcher.exe --dev-overlay
```

## Linux / Steam Deck

Use Proton: `protontricks-launch --appid 45760 Launcher.exe` from this folder.
