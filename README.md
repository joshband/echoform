# Echoform

Echoform is a JUCE-based stereo memory-delay AU/VST3 plug-in inspired by the Chase Bliss Habit. It features dual playheads, automatic scanning, deterministic randomness, and a modifier chain that can be configured into different feedback and stereo behaviors.

## Features

- Dual playheads with manual scan and automatic wander
- Deterministic random modulation (seeded by randomSeed + host transport position)
- Modifier chain: wow/flutter, dropout, low-pass, pitch drift
- Feedback modes: Collect, Feed, Closed
- Routing modes: In, Out, Feed (per bank)
- 3-minute memory buffer with size-scaled scan/spread
- Stereo modes: Independent, Linked, Cross
- Token-based LookAndFeel loaded from `resources/visualdna_tokens.json`
- Inspect mode with a non-literal memory timeline and playhead positions

## Parameters

- `mix`: Dry/wet mix (0 = dry, 1 = wet)
- `scan`: Manual scan depth (0 = now, 1 = max delay)
- `scanMode`: Manual / Auto
- `autoScanRate`: Automatic scan rate in Hz (0 = manual only)
- `spread`: Normalized offset between playheads (scaled by size)
- `feedback`: Feedback amount (clamped for stability)
- `size`: Maximum delay length in seconds (0.05 - 60)
- `bankA_mod1..3`: Bank A modifiers (mod1 wow/flutter, mod2 dropout, mod3 tone)
- `bankB_mod1..3`: Bank B modifiers (mod1 wow/flutter, mod2 dropout, mod3 tone)
- `character`: Macro controlling modifier intensity
- `stereoMode`: Independent / Linked / Cross
- `mode`: Collect / Feed / Closed (Feed recirculates the processed effect; Closed loops full output)
- `routingA`: In / Out / Feed (controls where Bank A applies)
- `routingB`: In / Out / Feed (controls where Bank B applies)
- `always`: Record while bypassed
- `wipe`: Audition memory directly (wet only)
- `dryKill`: Disable the dry path
- `latch`: Freeze playback/write (hidden)
- `trails`: Allow wet tails while bypassed (hidden)
- `memoryDry`: Force dry-only while bypassed even if trails (hidden)
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

## QA Test Harness

Echoform includes comprehensive QA scenarios using the audio-dsp-qa-harness:

- **42 scenarios** covering feedback modes, stereo modes, modulation, routing, spatial acoustics, stress tests, and contract checks
- **Latest local run**: 41 passing, 1 warning, 0 failures
- **Native stimulus types** including room simulation for spatial testing
- **Contract pack adoption**: latency, parameter smoothing, and state integrity contracts are included in critical/comprehensive suites

### Test Categories

1. **Core Functionality**: Smoke test, determinism, buffer wraparound
2. **Feedback Modes**: Collect, Feed, Closed (3 scenarios)
3. **Stereo Modes**: Independent, Linked, Cross (3 scenarios + 1 spatial)
4. **Modulation**: Wow/flutter, dropout, tone (3 scenarios + 1 spatial)
5. **Routing**: Bank A/B input/output/feed combinations (5 scenarios)
6. **Parameter Sweeps**: Mix, scan, feedback, character (4 scenarios)
7. **Spatial Acoustics**: Feedback decay, tape mode ambience, cross stereo (3 scenarios)
8. **Stress Tests**: Determinism, buffer extremes, feedback saturation (3 scenarios)

### Running Tests

```bash
# Run critical suite (8 scenarios)
./build_qa/echoform_qa_artefacts/Release/echoform_qa scenarios/echoform_critical_suite.json

# Run comprehensive suite (42 scenarios)
./build_qa/echoform_qa_artefacts/Release/echoform_qa scenarios/echoform_comprehensive_suite.json

# Run individual scenario
./build_qa/echoform_qa_artefacts/Release/echoform_qa scenarios/echoform/smoke_test.json

# Run with performance profiling
./build_qa/echoform_qa_artefacts/Release/echoform_qa scenarios/echoform_performance_suite.json --enable-profiling
```

State integrity currently uses a proxy contract because the embedded harness snapshot in this repo does not yet expose the newer native state roundtrip APIs. This scenario should be migrated to true `state_roundtrip` once the harness snapshot is updated.

See `docs/SPATIAL_SCENARIOS_RETROACTIVE_IMPROVEMENTS.md` for details on the 3 new spatial scenarios that test echoform with room acoustics.

## Legacy Determinism Test

`src/DeterminismTest.cpp` registers a JUCE `UnitTest` that processes identical input twice with the same seed and asserts bit-identical results. Run it from a JUCE unit test runner if you wire one into your host or standalone app.
