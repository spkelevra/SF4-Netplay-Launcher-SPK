# SF4 Enhanced v0.1.0-testers

Rollback netplay for **Ultra Street Fighter IV** (Steam) with a modern **SF4 Enhanced** launcher.

## Based on sf4e

This build is a fork of [sf4e](https://codeberg.org/adanducci/sf4e) by Anthony D'Anducci and contributors (MIT). See `ATTRIBUTION.md` in the zip.

## Prerequisites

- **Ultra Street Fighter IV** on Steam (app 45760) — not included
- [Microsoft Edge WebView2 Runtime](https://go.microsoft.com/fwlink/p/?LinkId=2124703)
- [VC++ Redistributable (x86)](https://aka.ms/vs/17/release/vc_redist.x86.exe)

## Install

1. Download the **team zip** asset below (not "Source code" only).
2. Extract the **entire** zip to one folder.
3. Run `powershell -ExecutionPolicy Bypass -File preflight.ps1`
4. Run `Launcher.exe` — **Host**, **Join**, or **Offline**

## Netplay quick test (LAN)

| Host | Joiner |
|------|--------|
| Host → Copy room code | Join → paste `IP:port` |
| Both Ready in-game | Same zip on both PCs |

Host may need port **23456** forwarded for internet play.

## Known limitations

- Manual IP / port-forward for WAN; no matchmaking or auto-NAT yet
- Both players must use the **same release zip** (`Sidecar.dll` must match)

## Support

Include the **Git** line from `BUILD_INFO.txt` and a screenshot when reporting issues.
