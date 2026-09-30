# Finger Drag — Geode Android

Target: Geometry Dash 2.2081 / Geode 5.10.1.

This is a source project, not a precompiled `.geode` binary.

## Build

Install the Geode SDK and Android NDK, then run:

    geode sdk install-binaries -p android64
    geode build -p android64

The resulting `.geode` package is produced in the Android build directory. Copy that package to the Geode mods directory on the phone.

## Behavior

The mod hooks `LevelEditorLayer`, not `PlayLayer`, because the requested feature is for moving objects in the level editor. It installs a targeted touch delegate with high priority, finds a `GameObject` under the finger, preserves the initial grab offset, and updates the object's position while the finger moves.

The mod intentionally does not use mouse/keyboard input or the built-in Input Trigger.
