# Echoform Plugin Guide

## Overview

Echoform is a stereo memory-delay AU/VST3 plug-in with dual playheads, automatic scanning, deterministic randomness, and a character-driven modifier chain. It supports multiple stereo behaviors and feedback modes, plus an Inspect view for visualizing memory activity.

Memory buffer length is 180 seconds. SIZE ranges from 50 ms to 60 seconds.

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
Primary controls for mix, scan, scan mode, auto scan, spread, feedback, size, character, Bank A/B modifiers, routing A/B, stereo mode, feedback mode, always, wipe, dry kill, and random seed.

### Inspect Mode
Read-only timeline visualization of memory energy and playhead positions. Toggle using the Inspect button.

## Parameters and Value Definitions

| Parameter | Range | Units | Default | Definition |
| --- | --- | --- | --- | --- |
| mix | 0.0 to 1.0 | ratio | 0.5 | Dry/wet mix. 0 = dry only, 1 = wet only. |
| scan | 0.0 to 1.0 | normalized | 0.0 | Manual scan depth (continuous). 0 = most recent, 1 = max delay. |
| scanMode | Manual / Auto | enum | Auto | Manual uses scan as a fixed position. Auto wanders into memory and returns to real time. |
| autoScanRate | 0.0 to 2.0 | Hz | 0.0 | Automatic scan rate. 0 disables auto scan (manual only). |
| spread | 0.0 to 1.0 | normalized | 0.0 | Continuous offset between playheads, scaled by size. |
| feedback | 0.0 to 0.99 | ratio | 0.0 | Feedback amount. Internally clamped to 0.98 with soft saturation to prevent runaway gain. |
| size | 0.05 to 60.0 | seconds | 1.0 | Maximum delay length. |
| bankA_mod1 | -1.0 to 1.0 | bipolar | 0.0 | Bank A modifier 1 (wow/flutter). |
| bankA_mod2 | -1.0 to 1.0 | bipolar | 0.0 | Bank A modifier 2 (dropout). |
| bankA_mod3 | -1.0 to 1.0 | bipolar | 0.0 | Bank A modifier 3 (tone/low-pass). |
| bankB_mod1 | -1.0 to 1.0 | bipolar | 0.0 | Bank B modifier 1 (wow/flutter). |
| bankB_mod2 | -1.0 to 1.0 | bipolar | 0.0 | Bank B modifier 2 (dropout). |
| bankB_mod3 | -1.0 to 1.0 | bipolar | 0.0 | Bank B modifier 3 (tone/low-pass). |
| character | 0.0 to 1.0 | macro | 0.0 | Macro controlling modifier intensity (wow/flutter, dropout, tone, subtle drift). |
| stereoMode | Independent / Linked / Cross | enum | Independent | Independent = per-channel memory, Linked = mirrored reads, Cross = left reads right and vice versa. |
| mode | Collect / Feed / Closed | enum | Feed | Collect = input only writes to memory. Feed = processed effect output feeds memory (post playback modifiers, no dry). Closed = full output feeds memory. |
| routingA | In / Out / Feed | enum | Out | In applies Bank A on write-in, Out applies on playback, Feed applies on feedback. |
| routingB | In / Out / Feed | enum | Out | In applies Bank B on write-in, Out applies on playback, Feed applies on feedback. |
| always | false / true | bool | false | Record while bypassed. |
| wipe | false / true | bool | false | Audition memory directly without input. |
| dryKill | false / true | bool | false | Remove the dry path. |
| latch | false / true | bool | false | Freeze playback/write (hidden). |
| trails | false / true | bool | false | Allow wet tails while bypassed (hidden). |
| memoryDry | false / true | bool | false | Force dry-only while bypassed, even if trails (hidden). |
| bypass | false / true | bool | false | Host bypass state (hidden). |
| randomSeed | 0 to 65535 | int | 1 | Seed for deterministic randomness. Combined with host transport position for repeatable offline renders. |

Scan and spread scale continuously with size (no quantization). In Feed mode, the recirculating signal is the processed effect
(post playback modifiers, no dry), so modifier changes accumulate while size/scan/spread only change read positions.

## Determinism

All random modulation uses a deterministic generator seeded by `randomSeed` plus host transport position. Offline renders with the same seed and transport position are bit-identical.

## Troubleshooting

- If the plug-in does not appear, rescan in your host or restart the DAW.
- If the UI looks default, confirm `resources/visualdna_tokens.json` is present.
