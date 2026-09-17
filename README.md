# Nonte

```
    ████████████████████████████████  
    ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀██████▀▀▀▀▀▀    NONTE
                        ██████          Modern, lightweight terminal code editor
          ▄▄██████▄▄    ██████          built on GNU nano.
        ▄████████████▄  ██████          
      ▄████████████████▄██████          [>] Version : 0.1.0-dev
     ▄██████▀    ▀████████████          [*] Base    : GNU nano
     ██████        ███████████          [=] License : GPL v3+
     ██████   ▄██▄  ██████████          </> Language: C
     ▀██████▄▄████   █████████          
       ▀████████▀    █████████          
          ▀▀▀▀▀      █████████          
                     █████████          
                     █████████          
                   ▄██████████▄         
```

**Nonte** is a modern, fast, and feature-rich terminal code editor based on GNU nano. It turns the traditional nano editor into a full-fledged IDE-like editing experience with modern keyboard shortcuts, mouse text selection, fuzzy file finders, auto-closing brackets, git diff indicators, and an interactive command palette—all while maintaining the instantaneous startup speed and zero-dependency footprint of nano.

---

## Highlights & Features

* **VS Code-Style Mouse & Text Selection**:
  * Click & drag text selection, double-click for word selection, triple-click for line selection.
  * Selection replacement: typing replaces selected text; `Backspace` or `Delete` deletes the selection.
  * Standard clipboard keys: `Ctrl+C` (copy), `Ctrl+X` (cut), and `Ctrl+V` (paste).

* **Command Palette (`Ctrl+Shift+P` / `F1` / `Alt+P`)**:
  * Searchable modal listing every available editor command, description, and keyboard shortcut.
  * Run any action immediately without having to memorize shortcuts.

* **Quick Open File (`Ctrl+P`)**:
  * Fuzzy finder modal scanning your workspace files in real time.
  * Navigate with `Up`/`Down` and press `Enter` to open in a buffer tab.

* **Project-Wide Grep / Find in Files (`Ctrl+Shift+F` / `Alt+F`)**:
  * Recursive codebase search with matching line numbers and snippets.
  * Press `Enter` on any search result to jump directly to the exact file, line, and column.

* **Multi-Cursor / Next Occurrence (`Ctrl+D`)**:
  * When no text is selected: selects the word under cursor.
  * When text is selected: finds and highlights the next occurrence in the buffer.

* **Line Movement & Duplication**:
  * `Alt+Up` / `Alt+Down`: Move current line or selected block up and down.
  * `Shift+Alt+Down` / `Ctrl+Shift+D`: Duplicate current line or selected block down.

* **Bracket & Quote Auto-Close + Highlight**:
  * Automatically closes `()`, `[]`, `{}`, `""`, `''`, and ```` `` ```` with intelligent step-over when typing closing characters.
  * Backspace between empty pairs removes both opening and closing characters.
  * Real-time visual pair highlight on the matching bracket.

* **Split View (`Ctrl+\`)**:
  * Side-by-side dual buffer view separated by a clean vertical divider.

* **Git Gutter Indicators**:
  * Colored diff markers in the margin column (`+` in green for added lines, `~` in yellow for modified lines).

* **Tabs & Workspace Explorer**:
  * Clean buffer tab bar across the top (`Alt+,` / `Alt+.`, `Ctrl+Tab`, `Ctrl+Shift+Tab`).
  * File Explorer sidebar (`F2` / Command Palette) with interactive file and directory tree browsing.

* **Fast Autocomplete**:
  * Non-intrusive popup menu offering language keywords, standard library symbols, and buffer identifiers.

* **Auto-Trim Whitespace**:
  * Automatically strips trailing spaces and tabs from all lines on save (`Ctrl+S`).

---

## Keyboard Shortcuts

| Shortcut | Action | Description |
| :--- | :--- | :--- |
| `Ctrl+Shift+P` / `F1` | **Command Palette** | Search and run any editor command |
| `Ctrl+P` | **Quick Open** | Fuzzy search and open workspace files |
| `Ctrl+Shift+F` / `Alt+F` | **Find in Files** | Search across all files in repository |
| `Ctrl+D` | **Next Occurrence** | Select word under cursor / jump next match |
| `Alt+Up` / `Alt+Down` | **Move Line** | Move current line or selection up / down |
| `Shift+Alt+Down` | **Duplicate Line** | Duplicate line or selection downward |
| `Ctrl+/` / `Alt+/` | **Toggle Comment** | Comment or uncomment line / selection |
| `Ctrl+\` | **Split View** | Toggle side-by-side split screen |
| `Ctrl+S` | **Save File** | Save buffer & auto-trim whitespace |
| `Ctrl+Z` / `Ctrl+Y` | **Undo / Redo** | Undo or redo editing changes |
| `Ctrl+C` / `Ctrl+X` / `Ctrl+V` | **Clipboard** | Copy, cut, and paste selection |
| `Alt+,` / `Alt+.` | **Buffer Tabs** | Switch to previous / next open tab |
| `Ctrl+Tab` / `Ctrl+Shift+Tab` | **Tab Switcher** | Next / previous buffer tab |
| `F2` | **File Explorer** | Toggle workspace file explorer sidebar |
| `Ctrl+Q` | **Quit** | Exit Nonte |

---

## Building from Source

### Prerequisites

On Debian / Ubuntu:
```bash
sudo apt update
sudo apt install build-essential autoconf automake autopoint gettext pkg-config libncursesw5-dev libmagic-dev zlib1g-dev
```

On Fedora / RHEL:
```bash
sudo dnf install gcc make autoconf automake gettext-devel ncurses-devel file-devel zlib-devel
```

On Arch Linux:
```bash
sudo pacman -S base-devel ncurses file zlib
```

### Build & Install

```bash
git clone https://github.com/<your-username>/nonte.git
cd nonte
./autogen.sh
./configure --enable-utf8
make -j$(nproc)
sudo make install
```

Run Nonte:
```bash
nonte
```

---

## License

Nonte is based on [GNU nano](https://www.nano-editor.org/) and is licensed under the **GNU General Public License v3.0 or later** ([GPL-3.0-or-later](COPYING)).
