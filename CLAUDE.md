# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
# From the build directory:
qmake ../f5_17.pro && make      # Linux/macOS
qmake ../f5_17.pro && jom       # Windows with MSVC (jom is the nmake replacement)
```

The project uses Qt 5.15.2, MSVC 2019 64-bit, C++17. Pre-built Makefile exists at `build/Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/Makefile`.

Open in Qt Creator 18 via `f5_17.pro` for IDE build/debug.

## Architecture

This is a **video meeting / chat client** (Qt Widgets application). Currently in early development — the UI shell is in place but networking, video, and audio are not yet implemented.

| File | Role |
|------|------|
| `main.cpp` | Entry point — creates `QApplication` and `Widget` |
| `widget.h/cpp` | Main window — server connection, IP/port validation, meeting controls, chat tab |
| `widget.ui` | Qt Designer XML — full layout with all named widgets |
| `chatmessage.h/cpp` | Custom `QWidget` subclass — paints a rounded-rect chat bubble via `paintEvent` |

### UI Layout (top to bottom)

1. **Top bar** — Create meeting, Join meeting (by ID), Connect to server (IP + port with validators)
2. **Main area** — Left: `QTabWidget` (user list tab + chat tab with message list, text input, send button). Right: main screen area (placeholder for video)
3. **Bottom controls** — Camera toggle, Audio toggle + volume slider, Exit meeting
4. **Footer** — Log output label (`out_log`)

### Key widget names (from `widget.ui`)

Server: `ip_edit`, `port_edit`, `connect_btn`
Meeting: `create_meeting_btn`, `join_meeting_btn`, `meeting_no_edit`, `exit_meeting_btn`
Chat: `msg_list_widget` (QListWidget), `send_msg_btn`, `plainTextEdit`
Media: `open_camera_btn`, `open_audio_btn`, `audio_slider`
Other: `partner_list_widget`, `out_log`, `tabWidget`

### Code conventions

- 4-space indentation, spaces for tabs
- UTF-8 source encoding (enforced via `/utf-8` MSVC flag)
- UI strings are in Chinese
- Generated files (`ui_widget.h`, `moc_*.cpp`) are git-ignored — they're produced by Qt's UIC and MOC tools during build

### `Widget` slots (currently wired)

- `connect_to_server()` — validates IP (regex) and port (range), then shows a confirmation `QMessageBox` (no actual socket connection yet)
- `send_msg()` — creates a `ChatMessage` widget and adds it to `msg_list_widget` as a `QListWidgetItem`

### `ChatMessage` custom widget

Fixed size 400×40. Paints a green rounded rectangle with placeholder text "这是一条消息". Designed to be embedded into `QListWidget` items for chat bubble rendering.
