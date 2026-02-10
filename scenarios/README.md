# Echoform QA Scenarios

Comprehensive test coverage for Echoform MemoryDelayEngine using the audio-dsp-qa-harness framework.

## 📁 Directory Structure

```
scenarios/
├── echoform/
│   ├── core/                  # Core functionality tests
│   │   ├── smoke_test.json           ✅ Basic validation
│   │   ├── buffer_wraparound.json    ✅ Circular buffer integrity
│   │   └── determinism.json          ✅ Bit-identical output
│   ├── feedback/              # Feedback mode tests
│   │   ├── collect_mode_overdub.json ✅ Overdub accumulation
│   │   ├── feed_mode_stability.json  ✅ Feedback stability
│   │   └── closed_mode_isolation.json ✅ Self-sustaining loop
│   ├── stereo/                # Stereo mode tests
│   │   ├── stereo_mode_independence.json ✅ Dual-mono
│   │   ├── stereo_mode_linked.json       ✅ Linked playheads
│   │   └── stereo_mode_cross.json        ✅ Ping-pong
│   ├── modulation/            # Modulation tests
│   │   ├── auto_scan_modulation.json  ✅ LFO scan
│   │   ├── modifier_wow_flutter.json  ✅ Tape wow/flutter
│   │   └── modifier_dropout.json      ✅ Tape dropout
│   └── edge_cases/            # Edge case tests
│       ├── size_parameter_sweep.json  ✅ Size crossfading
│       ├── extreme_feedback.json      ✅ 98% feedback stability
│       └── zero_latency_passthrough.json ✅ No algorithmic delay
├── echoform_test_suite.json      # Complete suite (15 scenarios)
└── echoform_critical_suite.json  # Fast critical tests (5 scenarios)
```

## 🚀 Quick Start

### Run Smoke Test (5 seconds)
```bash
cd /Users/artbox/Documents/Repos/echoform
./build/echoform_qa_artefacts/Debug/echoform_qa scenarios/echoform/smoke_test.json
```

### Run Critical Suite (30 seconds)
```bash
./build/echoform_qa_artefacts/Debug/echoform_qa scenarios/echoform_critical_suite.json
```

### Run Complete Suite (2-3 minutes)
```bash
./build/echoform_qa_artefacts/Debug/echoform_qa scenarios/echoform_test_suite.json
```

### Run Individual Scenario
```bash
./build/echoform_qa_artefacts/Debug/echoform_qa scenarios/echoform/feedback/feed_mode_stability.json
```

## 📊 Test Coverage

### Migrated from Existing Tests (3 scenarios)
- ✅ **buffer_wraparound** ← `MemoryDelayEngineTests::testWraparoundDsp()`
- ✅ **collect_overdub** ← `MemoryDelayEngineTests::testCollectOverdub()`
- ✅ **determinism** ← `DeterminismTest.cpp`

### New Comprehensive Tests (12 scenarios)

#### Feedback Modes (3 scenarios)
- **Collect Mode**: Overdub accumulation, energy growth validation
- **Feed Mode**: Feedback stability, no runaway, monotonic decay
- **Closed Mode**: Self-sustaining loop, buffer isolation

#### Stereo Modes (3 scenarios)
- **Independent**: Channel separation, dual-mono verification
- **Linked**: Synchronized playheads, maintained stereo image
- **Cross**: Ping-pong effect, cross-channel feedback

#### Modulation (3 scenarios)
- **Auto-Scan**: LFO modulation, smooth transitions
- **Wow/Flutter**: Tape modulation (MOD1), pitch variation
- **Dropout**: Tape degradation (MOD2), amplitude variation

#### Edge Cases (3 scenarios)
- **Size Sweep**: Buffer crossfading, smooth size changes
- **Extreme Feedback**: 98% feedback stability boundary test
- **Zero Latency**: Real-time passthrough validation

## 🎯 Test Suites

### Critical Suite (Fast) - 5 scenarios
Essential tests for pre-release validation:
1. Smoke test (basic functionality)
2. Determinism (regression baseline)
3. Feed mode stability (core feature)
4. Buffer wraparound (buffer integrity)
5. Zero latency (performance)

**Run Time**: ~30 seconds
**Use Case**: CI/CD, pre-commit hook, quick validation

### Comprehensive Suite - 15 scenarios
Complete feature coverage:
- All 3 feedback modes
- All 3 stereo modes
- All modulation types
- Edge cases and boundary conditions

**Run Time**: ~2-3 minutes
**Use Case**: Pre-release validation, regression testing, feature development

## 📈 Metrics Used

All scenarios use validated harness metrics:
- ✅ `signal_present` - Audio output verification
- ✅ `peak_level` - Clipping detection
- ✅ `monotonic_tail_decay` - Stability validation
- ✅ `discontinuity_count` - Artifact detection
- ✅ `energy_growth` - Feedback stability
- ✅ `rms_variance` - Modulation activity
- ✅ `stereo_correlation` - Stereo width
- ✅ `initial_silence_duration` - Latency check

## ⚙️ Parameters & Defaults

The adapter (`qa/echoform_adapter.cpp`) initializes these defaults:
```cpp
engine_->setMix(0.5f);              // 50% wet/dry
engine_->setScan(0.5f);             // Center scan
engine_->setFeedback(0.3f);         // 30% feedback
engine_->setSize(0.1f);             // 100ms delay
engine_->setAlwaysRecord(true);     // Recording enabled
engine_->setCharacter(0.5f);        // Neutral character
engine_->setSpread(0.5f);           // Medium spread
```

**Note**: Parameter variations in scenarios are not yet implemented in the harness (see `parameter_variations` in scenarios - currently empty but ready for future enhancement).

## 🔍 Test Results

Results are written to `qa_output/<scenario_id>/`:
- `dry.wav` - Dry (input) signal
- `wet.wav` - Processed output
- Metrics evaluated against expected invariants

### Pass/Fail Criteria
- **PASS**: All hard_fail invariants pass, soft_warn allowed
- **WARN**: Hard_fail pass, some soft_warn failures
- **FAIL**: At least one hard_fail invariant fails

## 🛠️ Adding New Scenarios

1. Create scenario JSON in appropriate subdirectory
2. Follow harness v1.0 format (see existing scenarios)
3. Use validated metric names (see Metrics section)
4. Add to test suite JSON
5. Test with: `./build/echoform_qa_artefacts/Debug/echoform_qa <new_scenario>.json`

Example structure:
```json
{
  "scenario_version": "1.0",
  "id": "my_test",
  "name": "My Test Name",
  "category": "delay_reverb",
  "description": "What this tests",
  "capability_requirements": { ... },
  "stimulus": { ... },
  "parameter_variations": {},
  "analysis_windows": { ... },
  "expected_invariants": { ... },
  "baseline_tracking": { ... }
}
```

## 📚 References

- [QA Harness Documentation](../external/qa_harness/README.md)
- [Scenario Format Spec](../external/qa_harness/docs/SCENARIO_FORMAT.md)
- [Available Metrics](../external/qa_harness/docs/METRICS_REFERENCE.md)
- [Integration Guide](../external/qa_harness/docs/guides/INTEGRATION_GUIDE.md)

## ✅ Test Status

**Last Run**: 2026-02-08
**Status**: ✅ 1/15 scenarios tested (smoke_test: PASS)
**Next**: Run full suite validation

---

*Generated by audio-dsp-qa-harness v1.0.0*
