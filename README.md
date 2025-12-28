# Echoform

Echoform is a JUCE-based stereo memory-delay AU/VST3 plug-in inspired by the Chase Bliss Habit. It features dual playheads, automatic scanning, deterministic randomness, and a modifier chain that can be configured into different feedback and stereo behaviors.

## Features

- Dual playheads with manual scan and automatic wander
- Deterministic random modulation (seeded by randomSeed + host transport position)
- Modifier chain: low-pass, pitch drift, wow/flutter, dropout
- Feedback modes: Collect, Feed, Closed
- Stereo modes: Independent, Linked, Cross
- Token-based LookAndFeel loaded from `resources/visualdna_tokens.json`
- Inspect mode with a non-literal memory timeline and playhead positions

## Parameters

- `mix`: Dry/wet mix (0 = dry, 1 = wet)
- `scan`: Manual scan depth (0 = now, 1 = max delay)
- `autoScanRate`: Automatic scan rate in Hz (0 = manual only)
- `spread`: Offset between playheads in seconds
- `feedback`: Feedback amount (clamped for stability)
- `time`: Maximum delay length in seconds
- `character`: Macro controlling modifier intensity
- `stereoMode`: Independent / Linked / Cross
- `mode`: Collect / Feed / Closed (Feed keeps modifiers static; Closed loops full output)
- `randomSeed`: Seed for deterministic randomness

## Build

1. Install JUCE and update `JUCE_DIR` in `CMakeLists.txt`.
2. Configure the build:

```sh
cmake -S . -B build
```

3. Build the plug-in:

```sh
cmake --build build
```

## Design Tokens

The UI reads tokens from `resources/visualdna_tokens.json`. If the file is missing or incomplete, the plug-in falls back to built-in defaults.

## Determinism Test Harness

`src/DeterminismTest.cpp` registers a JUCE `UnitTest` that processes identical input twice with the same seed and asserts bit-identical results. Run it from a JUCE unit test runner if you wire one into your host or standalone app.
