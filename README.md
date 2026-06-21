# MIDISequencer

Declarative event-driven MIDI step sequencer written in C++.
## Prerequisites (macOS)

```bash
brew install cmake ninja sdl3
```

`rtmidi` and `imgui` are pulled in automatically via CMake `FetchContent`.

## Build & run

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/MIDISequencer
```
