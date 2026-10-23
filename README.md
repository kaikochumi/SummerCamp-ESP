# SummerCamp ESP

![license](https://img.shields.io/badge/license-MIT-blue)
![language](https://img.shields.io/badge/language-C%2B%2B-00599C)
![platform](https://img.shields.io/badge/platform-Windows-0078D6)
![release](https://img.shields.io/github/v/release/kaikochumi/SummerCamp-ESP)
![build](https://github.com/kaikochumi/SummerCamp-ESP/actions/workflows/build.yml/badge.svg)

External ESP overlay for **Friday the 13th: The Game** — the original
*Summer Camp* build set at Camp Forest Green (Unreal Engine 4.18.3). It
renders player boxes, skeletons and health bars in a transparent
topmost window. The game process is opened with read access only:
nothing is written, no injection, no driver.

The menu ships in English and Japanese (toggle in `Settings`, or set
`lang` in the config file).

![In game](docs/screenshots/in-game.png)
*Playing as Jason: four counselors tracked with corner boxes,
skeletons, snaplines and distances — menu open on the right.*

> [!IMPORTANT]
> **Does it draw? Drop a ⭐.** This is a hobby project — the star
> counter is the only signal that people actually use the tool, and it
> is what pushes the next offset update out when the game changes.

## Screenshots

| English | Japanese |
|---------|----------|
| ![Menu in English](docs/screenshots/menu-en.png) | ![Menu in Japanese](docs/screenshots/menu-ja.png) |

## Features

- Player ESP: box in solid, corner or rounded style, skeleton, name,
  distance, health and stamina bars
- Snapline, head marker and center crosshair, each togglable
- Target detection is automatic: as a counselor you track Jason; as
  Jason you track the counselors
- Targets tab with a live table (name, distance, health, visibility)
- Everything is stored in a config file and survives restarts
- Click-through overlay: only the menu accepts the mouse, the game
  keeps the rest of the input

## Requirements

Windows 10 or newer and CMake 3.20+. One compiler:

- MinGW-w64 (tested with GCC 16)
- MSVC (Build Tools 2022 works unchanged)

Dear ImGui is vendored in `third_party/imgui`, so the build needs no
network access. Only the win32 and dx11 backends are kept.

## Build

With MinGW:

    cmake -B build -G "MinGW Makefiles"
    cmake --build build -j

With MSVC, omit the `-G` line and build the generated solution. The
executable ends up at `build/summercamp-esp.exe`.

## Usage

Grab `SummerCamp-ESP-v0.1.0-Win64.zip` from the
[latest release](https://github.com/kaikochumi/SummerCamp-ESP/releases/latest),
unzip it anywhere, launch the game first, then the tool:

    summercamp-esp.exe --module summercamp --cfg esp.cfg

Both flags are optional. `--module` is the module name used to find the
game process; `--cfg` points at the config file, which is created with
default values on the first run and saved again whenever you touch the
menu.

| Key    | Action      |
|--------|-------------|
| INSERT | toggle menu |
| END    | toggle ESP  |
| DELETE | quit        |

A preset tuned for the standard look (thin orange corner boxes,
skeleton, distance in meters) ships in `config/preset.cfg`:

    summercamp-esp.exe --cfg config/preset.cfg

## Configuration

The config is one `key=value` per line, same names as the menu items.
Colors use the IM_COL32 order (`0xAABBGGRR`). `lang=0` starts the menu
in English, `lang=1` in Japanese; the Japanese face is loaded from the
system fonts (Meiryo, then Yu Gothic, then MS Gothic).

## Offsets

The memory layout lives in `src/offsets.h` and is documented in
[docs/OFFSETS.md](docs/OFFSETS.md). Values were mapped from the engine
and game structures and confirmed against the live game (UE 4.18.3,
build 38951); other game versions will have different addresses.

**Game version:** tested against the community *Summer Camp* build
`4.18.3-38951` (Camp Forest Green launcher) as of **October 2026**. If
your game binary moves to a different build, the addresses will need to
be re-confirmed first.

## Roadmap

- Objectives ESP: car battery, gas, fuse, meds, pocket knife
- Per-role box colors (counselors vs Jason)
- Build notice in the menu when the game updates
- More translations for the menu

## Disclaimer

This project reads game memory from outside the process and exists for
study of Unreal Engine layouts. Use it at your own risk; the author is
not responsible for account penalties or other consequences.

## Support

If the ESP worked for you, a star is the best feedback — it is what
keeps this repo active and gets [docs/OFFSETS.md](docs/OFFSETS.md)
updated faster whenever the game changes.

⭐ [**Star kaikochumi/SummerCamp-ESP**](https://github.com/kaikochumi/SummerCamp-ESP/stargazers)

