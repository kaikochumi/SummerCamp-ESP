# Contributing

Small project, so the process is light.

## Bugs and ideas

Open an issue and include: the build of the game you tested against,
the exact behavior you saw, and what you expected. A screenshot helps a
lot for anything menu-related.

## Code

- C++17; no dependencies outside the vendored ImGui in
  `third_party/imgui`
- Keep changes small: one fix or one feature per pull request
- Match the surrounding style: 4-space indent, same naming patterns,
  no wholesale reformats
- If you touch memory offsets or the skeleton pipeline, read
  [docs/OFFSETS.md](docs/OFFSETS.md) first and note in the pull request
  how you verified the values against the running game

## Before opening a pull request

Build cleanly with MinGW:

    cmake -B build -G "MinGW Makefiles"
    cmake --build build -j

and run the tool with the game open, checking that the menu still
respects the config file (the screenshots in `docs/screenshots/` were
taken that way).
