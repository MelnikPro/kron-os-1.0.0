<div align="center">

# KRON Desktop Environment

**A minimal, fast Wayland desktop environment for Arch Linux**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Arch%20Linux-1793D1?logo=arch-linux)](https://archlinux.org)
[![Wayland](https://img.shields.io/badge/display-Wayland-orange)](https://wayland.freedesktop.org)
[![Language](https://img.shields.io/badge/language-C-555555?logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))

</div>

---

## What is KRON?

KRON is a lightweight desktop environment built from scratch for Arch Linux using:

- **[wlroots](https://gitlab.freedesktop.org/wlroots/wlroots)** — Wayland compositor library (same foundation as Sway, Hyprland)
- **[GTK4](https://gtk.org)** — Shell UI (panel, launcher, notifications)
- **[gtk4-layer-shell](https://github.com/wmww/gtk4-layer-shell)** — Proper panel anchoring via Wayland layer-shell protocol

KRON consists of two components:

| Component | Description |
|-----------|-------------|
| `kron` | Wayland compositor — manages windows, input, rendering |
| `kron-shell` | GTK4 shell — panel, app launcher, notifications |

---

## Features

- Native Wayland compositor (no X11 required)
- Panel anchored to screen edge via `wlr-layer-shell`
- App launcher with search
- Taskbar showing open windows via `wlr-foreign-toplevel`
- Desktop notifications (implements `org.freedesktop.Notifications` DBus API)
- Server-side window decorations via `xdg-decoration`
- Monitor configuration support via `wlr-output-management`
- Move and resize windows with mouse
- Dark theme with accent color

---

## Requirements

| Package | Purpose |
|---------|---------|
| `wlroots` | Compositor backend |
| `gtk4` | Shell UI toolkit |
| `gtk4-layer-shell` | Layer-shell protocol for panel |
| `libxkbcommon` | Keyboard handling |
| `wayland` | Wayland protocol |
| `wayland-protocols` | Protocol extensions |
| `json-glib` | Config file parsing |
| `meson` + `ninja` | Build system |

---

## Installation

### Arch Linux

```bash
# Install dependencies
sudo pacman -S gtk4 gtk4-layer-shell libxkbcommon wayland wayland-protocols \
               json-glib meson ninja git base-devel

# Install wlroots (from AUR)
git clone https://aur.archlinux.org/yay.git && cd yay && makepkg -si && cd ..
yay -S wlroots

# Clone and build KRON
git clone https://github.com/MelnikPro/kron-os-1.0.0.git
cd kron-os-1.0.0/kron
meson setup build
ninja -C build
sudo ninja -C build install
```

### Start KRON

Select **KRON** from your display manager (GDM, SDDM, LightDM), or start from TTY:

```bash
kron
```

---

## Configuration

Config file: `~/.config/kron/config.json`

```json
{
  "theme": "dark",
  "accent_color": "#5294E2",
  "panel_height": 36,
  "panel_position": "top",
  "wallpaper": "/path/to/wallpaper.jpg",
  "wm": "kron",
  "autostart": [
    "nm-applet",
    "picom"
  ]
}
```

| Option | Values | Description |
|--------|--------|-------------|
| `theme` | `dark` / `light` | UI color scheme |
| `accent_color` | hex color | Accent color for highlights |
| `panel_height` | integer | Panel height in pixels |
| `wallpaper` | file path | Path to wallpaper image |
| `autostart` | array of commands | Programs to launch on startup |

---

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Alt + Esc` | Exit KRON session |
| `Alt + F1` | Switch focus to next window |

---

## Project Structure

```
kron/
├── compositor/          # Wayland compositor (C + wlroots)
│   ├── main.c           # Entry point
│   ├── server.c/h       # Core compositor server
│   ├── output.c/h       # Monitor management
│   ├── toplevel.c/h     # Window management (xdg-shell)
│   └── input.c/h        # Keyboard and mouse handling
├── shell/               # GTK4 shell
│   ├── main.c           # Shell entry point
│   ├── panel.c/h        # Top panel
│   ├── launcher.c/h     # Application launcher
│   ├── taskbar.c/h      # Taskbar (foreign-toplevel)
│   └── notifications.c/h # Desktop notifications (DBus)
├── protocols/           # Wayland protocol XML files
├── data/                # Desktop entry, CSS styles
└── meson.build          # Build configuration
```

---

## Roadmap

- [x] Wayland compositor
- [x] Window move and resize
- [x] Panel with clock
- [x] Application launcher
- [x] Taskbar
- [x] Desktop notifications
- [x] Server-side decorations
- [x] Monitor management
- [ ] XWayland support (X11 apps)
- [ ] System tray (StatusNotifierItem)
- [ ] Screen lock
- [ ] Screenshot tool
- [ ] Settings GUI

---

## Contributing

Pull requests are welcome. For major changes, open an issue first.

1. Fork the repository
2. Create your feature branch: `git checkout -b feature/my-feature`
3. Commit your changes: `git commit -m 'Add my feature'`
4. Push to the branch: `git push origin feature/my-feature`
5. Open a Pull Request

---

## License

[MIT](LICENSE) © 2026 MelnikPro
