# UI Contract

This document defines the UI contract for Echoform parameters exposed via `createParameterLayout()` in `src/PluginProcessor.cpp`. It must remain consistent with the parameter IDs, names, and ranges defined in code.

## Parameter Matrix

| Parameter ID | Name | Type | Range & Units | Default | Automation Support | Modulation Sensitivity | Intended UI Component | Interaction Priority |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `mix` | Mix | Continuous | 0.0–1.0 (ratio) | 0.5 | Supported (APVTS parameter) | High | Rotary knob/slider | High |
| `scan` | Scan | Continuous | 0.0–1.0 (normalized) | 0.0 | Supported (APVTS parameter) | High | Rotary knob/slider | High |
| `autoScanRate` | Auto Scan Rate | Continuous | 0.0–2.0 Hz | 0.0 | Supported (APVTS parameter) | Medium | Rotary knob/slider | Medium |
| `spread` | Spread | Continuous | -2.0–2.0 seconds | 0.0 | Supported (APVTS parameter) | Medium | Rotary knob/slider | Medium |
| `feedback` | Feedback | Continuous | 0.0–0.99 (ratio) | 0.0 | Supported (APVTS parameter) | High | Rotary knob/slider | High |
| `time` | Time | Continuous | 0.1–10.0 seconds | 1.0 | Supported (APVTS parameter) | Medium | Rotary knob/slider | High |
| `character` | Character | Continuous | 0.0–1.0 (normalized) | 0.0 | Supported (APVTS parameter) | Medium | Rotary knob/slider | Medium |
| `stereoMode` | Stereo Mode | Enum | Independent / Linked / Cross | Independent (index 0) | Supported (APVTS parameter) | Low | Combo box / segmented control | Medium |
| `mode` | Mode | Enum | Collect / Feed / Closed | Feed (index 1) | Supported (APVTS parameter) | Medium | Combo box / segmented control | Medium |
| `randomSeed` | Random Seed | Integer/Discrete | 0–65535 (integer) | 1 | Supported (APVTS parameter) | Low | Numeric slider/field | Low |

## Notes

- “Automation Support” reflects use of JUCE `AudioProcessorValueTreeState` parameters, which are automatable by hosts.
- “Modulation Sensitivity” and “Interaction Priority” are UI guidance only and do not imply DSP behavior beyond the exposed parameter ranges and names.
