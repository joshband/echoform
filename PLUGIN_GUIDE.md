# Echoform Plugin Guide

## Overview

Echoform is a stereo memory-delay AU/VST3 plug-in with dual playheads, automatic scanning, deterministic randomness, and a character-driven modifier chain. It supports multiple stereo behaviors and feedback modes, plus an Inspect view for visualizing memory activity.

Maximum delay time (and memory buffer length) is 10 seconds.

## Build and Install

### macOS (AU + VST3)
1. Point `JUCE_DIR` in `CMakeLists.txt` to your JUCE checkout.
2. Configure and build with Xcode:
```
cmake -S . -B build_xcode -G Xcode -DJUCE_DIR=../JUCE
cmake --build build_xcode --config Release
```

Artifacts (also installed automatically):
- VST3: `~/Library/Audio/Plug-Ins/VST3/Stereo Memory Delay.vst3`
- AU: `~/Library/Audio/Plug-Ins/Components/Stereo Memory Delay.component`

### Windows (VST3 only)
```
cmake -S . -B build
cmake --build build --config Release
```

### Notes
- Hosts may require a manual plug-in rescan after build.
- The UI loads tokens from `resources/visualdna_tokens.json`. If missing, defaults are used.

## Usage

### Play Mode
Primary controls for mix, scan, spread, feedback, time, character, auto scan, stereo mode, feedback mode, and random seed.

### Inspect Mode
Read-only timeline visualization of memory energy and playhead positions. Toggle using the Inspect button.

## Parameters and Value Definitions

| Parameter | Range | Units | Default | Definition |
| --- | --- | --- | --- | --- |
| mix | 0.0 to 1.0 | ratio | 0.5 | Dry/wet mix. 0 = dry only, 1 = wet only. |
| scan | 0.0 to 1.0 | normalized | 0.0 | Manual scan position. 0 = most recent, 1 = max delay. |
| autoScanRate | 0.0 to 2.0 | Hz | 0.0 | Automatic scan rate. 0 disables auto scan (manual only). |
| spread | -2.0 to 2.0 | seconds | 0.0 | Offset between primary and secondary playheads. Negative values place the secondary ahead. |
| feedback | 0.0 to 0.99 | ratio | 0.0 | Feedback amount. Internally clamped to 0.98 with soft saturation to prevent runaway gain. |
| time | 0.1 to 10.0 | seconds | 1.0 | Maximum delay length and buffer size. |
| character | 0.0 to 1.0 | macro | 0.0 | Macro controlling modifier intensity (low-pass, drift, wow/flutter, dropout). |
| stereoMode | Independent / Linked / Cross | enum | Independent | Independent = per-channel memory, Linked = mirrored reads, Cross = left reads right and vice versa. |
| mode | Collect / Feed / Closed | enum | Feed | Collect = input only writes to memory. Feed = raw effect output feeds memory (static modifiers). Closed = full output feeds memory (accumulative modifiers). |
| randomSeed | 0 to 65535 | int | 1 | Seed for deterministic randomness. Combined with host transport position for repeatable offline renders. |

## Determinism

All random modulation uses a deterministic generator seeded by `randomSeed` plus host transport position. Offline renders with the same seed and transport position are bit-identical.

## Troubleshooting

- If the plug-in does not appear, rescan in your host or restart the DAW.
- If the UI looks default, confirm `resources/visualdna_tokens.json` is present.
