# Ultra Street Fighter IV — sf4e netplay (player guide)



## Prerequisites



- **[Microsoft Edge WebView2 Runtime](https://go.microsoft.com/fwlink/p/?LinkId=2124703)** — required for the launcher UI (one-time install on Windows).

- **[VC++ Redistributable (x86)](https://aka.ms/vs/17/release/vc_redist.x86.exe)** — required for the sf4e binaries.



## Quick start



1. Extract a release build so `Launcher.exe`, `Sidecar.dll`, and the **`launcher-ui/`** folder sit in the same directory.

2. Run **Launcher.exe**.

3. In the WebView2 launcher, choose **Host**, **Join**, or **Offline**.

4. Click **Start game** — USF4 launches with netplay configured.



## Host



1. Select **Host**, enter display name and input delay.

2. The launcher shows your **LAN address** (same Wi‑Fi/Ethernet) and your **public internet address** (use **Refresh** if needed; you can edit it).

3. Click **Copy** on the room code and send it to your opponent (format `IP:port`, default port **23456**).

4. **Internet play:** joiners must use your **public** or **VPN** IP in the room code, not `192.168.x.x`, unless they are on the same LAN.

5. On your router, **port-forward TCP and UDP** on the session port to this PC. Allow the port in **Windows Firewall**.

6. Wait in the in-game lobby; when both players are **Ready** on the main menu, the match starts.



## Join



1. Select **Join**, enter your display name.

2. Paste the host’s room code into **Host address** (`IP:port` or `hostname:port`). The field remembers your last join.

3. You can use a **different IP** than the host’s LAN address (e.g. their public IP or a VPN IP).

4. Click **Start game**; the Network panel shows your **join target**. Press **Ready** in the lobby when connected.



## Offline



Launches USF4 with sf4e hooks but no netplay session.



## Command line (optional)



```bat

Launcher.exe --host

Launcher.exe --join 203.0.113.42:23456

Launcher.exe --offline

Launcher.exe --console

```



## Playing over the internet (WAN)



| Requirement | Notes |

|-------------|--------|

| Same `Sidecar.dll` on both PCs | Join fails with “version mismatch” otherwise |

| Host port-forward **23456** | Session port (changeable on the Host screen) |
| **WebView2 Runtime** | Launcher shows an install link if missing |

| Room code uses **reachable IP** | Public IP, or VPN IP (ZeroTier, Tailscale, etc.) |

| CGNAT / no public IP | Use a VPN; port-forward alone is not enough |



**Relay mode (default):** GGPO rollback traffic is tunneled through the session connection. You usually do **not** need to forward separate GGPO UDP ports. Set `SF4E_RELAY=0` only for advanced LAN troubleshooting.



## Build ID (version check)



Both players need the same `Sidecar.dll` build. Enable developer overlay (`SF4E_NETPLAY_DEV=1`) to see the build hash in-game.



## Graphics



Disable **Smooth** frame rate in in-game graphics options if rollback feels wrong.



## Logs



`Launcher.exe --console` or files under `%APPDATA%\sf4e\`.



## Linux / Steam Deck



Use `protontricks-launch --appid 45760 Launcher.exe` as described in the main README.

