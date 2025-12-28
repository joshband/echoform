# Habit-Informed JUCE Roadmap

This roadmap is based on the Chase Bliss Habit manual and is oriented to a JUCE
audio plugin implementation. Page references match the PDF.

## Manual Review Highlights (implementation-critical)

- 3 minute circular memory buffer that is always recording while the pedal is on.
  The memory is always 3 minutes long and acts like a digital tape loop.
  (pg. 12)
- Memory is cleared on bypass unless COLLECT or ALWAYS are enabled.
  (pg. 12, 22)
- SIZE sets the delay spacing and ranges from 50 ms to 60 seconds with no pitch
  warping during adjustment. (pg. 5)
- SCAN is dual-mode: AUTO (default) wanders into memory and returns to real time,
  while MANUAL selects a fixed position in memory. The knob sets both scan
  frequency and how far into memory the scan goes. (pg. 6, 13)
- SPREAD introduces a secondary echo (secondary playhead) that reads a different
  memory position but shares SIZE and REPEATS. SPREAD and SCAN are rhythmically
  linked to SIZE. (pg. 6, 16)
- MODIFY contains two banks (A/B) and three modifiers per bank; each modifier has
  a neutral center with different behavior on either side. (pg. 10)
- Routing toggle IN/OUT/FEED changes how modifiers accumulate:
  IN accumulates modifiers per echo, OUT keeps modifiers static, FEED routes
  output back to input for a closed loop. (pg. 7, 20)
- COLLECT overdubs the 3 minute memory and allows song building. WIPE bypasses
  the echo to audition memory directly. (pg. 18)
- FEED creates a closed loop where output feeds memory, causing evolving
  coloration and different REPEATS behavior. (pg. 20)
- Additional behaviors: ALWAYS (record in bypass), DRY KILL, LATCH loop behavior,
  MEMORY DRY and TRAILS as hidden options. (pg. 22, 23)
- External control and ramping concepts exist but can be staged for later phases.
  (pg. 24, 26)

## Architecture Targets

- MemoryBuffer: circular buffer sized to 3 minutes at sample rate, stereo.
- Playhead: primary and secondary read heads with manual or auto scan.
- ModifierChain: bank A/B with three modifier types each, neutral center behavior.
- Routing: IN/OUT/FEED paths controlling modifier accumulation and feedback.
- Mode: COLLECT (overdub), ALWAYS (record in bypass), WIPE (memory direct).
- Deterministic random: auto-scan wandering and modifier randomness.

## Phased Implementation Plan

### Phase 1 - Memory and Timing Core
- Set buffer length to 180 seconds and implement circular read/write.
- Implement SIZE range 50 ms to 60 s with pitch-safe changes.
- Ensure SCAN and SPREAD are linked to SIZE for rhythmic sync.

### Phase 2 - Scan and Spread Behavior
- Implement SCAN AUTO: periodic wander with depth and return-to-now behavior.
- Implement SCAN MANUAL: fixed read offset (pre-delay analogy).
- Implement SPREAD: secondary playhead offset linked to SCAN/SIZE.

### Phase 3 - Modify Banks and Routing
- Build modifier bank A/B and 1/2/3 selection.
- Add modifier accumulation modes:
  IN = accumulative, OUT = static, FEED = closed loop.
- Map modifier intensity with a neutral center and bipolar behavior.

### Phase 4 - Collect, Feed, and Bypass Logic
- Implement COLLECT overdub mode (memory keeps layering).
- Implement ALWAYS (record in bypass) and safety limiting in bypass.
- Implement WIPE (memory direct, no echo).
- Implement FEED closed loop behavior and REPEATS as system feedback.

### Phase 5 - UI, Inspect, and External Control
- Add toggles for MANUAL, COLLECT, ALWAYS, WIPE, DRY KILL, routing.
- Add Inspect view for memory activity and playhead positions.
- Stage ramping and external control (CV/MIDI) as optional expansion.

## JUCE Mapping Notes

- MemoryBuffer length: `sampleRate * 180` samples, allocate in prepareToPlay.
- SIZE: time parameter (0.05 to 60.0 seconds) mapped to playhead delay.
- SCAN AUTO: uses deterministic RNG, parameter controls rate and depth.
- SCAN MANUAL: parameter controls normalized read position.
- SPREAD: uses a second playhead offset, linked to SIZE for rhythmic sync.
- Routing and modes: expose as AudioParameterChoice and booleans.
- Modifier chain: map A/B banks to discrete choices, neutral midpoint behavior.

## CODEX Prompt Templates

### Prompt 1 - Memory and Size
```
Set the memory buffer to 3 minutes (180 seconds) using a circular buffer.
Update SIZE to range 50 ms to 60 s with pitch-safe changes. Ensure no
allocations in processBlock. Add a test that validates wraparound.
```

### Prompt 2 - Scan Modes
```
Implement SCAN with two modes: AUTO and MANUAL. AUTO should wander into
memory and return to real time; the knob sets both rate and depth. MANUAL
selects a fixed position in memory (pre-delay). Make scanning deterministic.
```

### Prompt 3 - Spread and Dual Playheads
```
Add a secondary playhead (SPREAD) that reads a different point in memory.
SPREAD and SCAN should be linked to SIZE for rhythmic sync. Secondary echo
shares SIZE and REPEATS with the primary.
```

### Prompt 4 - Modifier Banks and Routing
```
Add modifier banks A/B with three modifiers each. Provide neutral center
behavior with bipolar controls. Implement routing modes: IN (accumulative),
OUT (static), FEED (closed loop).
```

### Prompt 5 - Collect, Feed, and Wipe
```
Add COLLECT overdub mode, ALWAYS record-in-bypass, and WIPE to bypass the
echo and audition memory directly. Implement FEED so output re-enters the
memory, and make REPEATS control system feedback in FEED.
```

### Prompt 6 - UI and Inspect
```
Expose SCAN mode, SPREAD, routing, COLLECT, ALWAYS, and WIPE in the UI.
Add Inspect mode to visualize memory energy and playhead positions. Keep
Inspect read-only.
```

## Open Questions for Implementation

- Should SIZE and SCAN/SPREAD sync be quantized to divisions, or just scaled
  continuously with tempo-like relationships?
- Which modifiers should be prioritized to match the manual in early phases?
- Should FEED capture parameter changes over time in the buffer?
