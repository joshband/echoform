# Echoform Testing Guide

**Plugin**: Echoform v1.0 (Stereo Memory Delay)
**QA Framework**: [audio-dsp-qa-harness](https://github.com/joshband/audio-dsp-qa-harness)
**Authority Status**: **HARNESS-AUTHORITATIVE** (established 2026-02-13)

---

## QA Authority Policy

**audio-dsp-qa-harness is the sole authoritative DSP QA gate for echoform.**

This means:
- ✅ All DSP quality decisions come from harness scenarios and suites
- ✅ All DSP regression detection uses harness baseline workflows
- ✅ All CI blocking gates for DSP validation use harness runners
- ✅ New DSP tests MUST be added as harness scenarios (JSON), not ad-hoc scripts or C++ tests

**Non-DSP testing** (plugin format validation, UI regression, DAW compatibility) may exist outside harness scope if needed, but must be clearly labeled as non-DSP checks.

**Authority Transition Date**: 2026-02-13
**Reference**: [QA Authority Program](../audio-dsp-qa-harness/docs/migrations/QA_AUTHORITY_PROGRAM.md)

---

## Quick Start

### Running Tests

**Critical Suite** (PR gate, ~30 seconds):
```bash
cd build_qa
./echoform_qa ../scenarios/echoform_critical_suite.json
```

**Full Suite** (comprehensive validation, ~2 minutes):
```bash
./echoform_qa ../scenarios/echoform_comprehensive_suite.json
```

**Stress Suite** (boundary conditions, ~1 minute):
```bash
./echoform_qa ../scenarios/echoform_stress_suite.json
```

**Performance Suite** (RT-safety validation):
```bash
./echoform_qa ../scenarios/echoform_performance_suite.json
```

---

## Build Configuration

### Build QA Harness

```bash
cmake -B build_qa -DENABLE_QA_HARNESS=ON -DBUILD_QA_TESTS=ON
cmake --build build_qa --target echoform_qa
```

### Run with CTest

```bash
ctest --test-dir build_qa --output-on-failure
```

---

## Test Coverage

**Total Scenarios**: 38 passing
**Feature Coverage**: 98% (all 28 parameters tested across 11 categories)

### Scenario Categories

1. **Core** (3 scenarios)
   - Smoke test, determinism, buffer wraparound

2. **Feedback Modes** (3 scenarios)
   - Collect, feed, closed modes

3. **Stereo Modes** (3 scenarios)
   - Independent, linked, cross-channel

4. **Modulation** (4 scenarios)
   - Auto-scan, wow/flutter, dropout, tone modifier

5. **Edge Cases** (4 scenarios)
   - Size sweep, extreme feedback, zero latency, feedback stress

6. **Routing** (5 scenarios)
   - Bank A input/output/feed, Bank AB combination/cascade

7. **Spread** (3 scenarios)
   - Spread width, scan interaction, stereo imaging

8. **Tape Mode** (2 scenarios)
   - Stochastic behavior, variance validation

9. **Boolean Flags** (4 scenarios)
   - Wipe, dry_kill, latch, trails

10. **Parameter Sweeps** (4 scenarios)
    - Mix, scan, feedback, character parameter automation

11. **Stress Tests** (4 scenarios)
    - Buffer size extremes, parameter slew, feedback saturation, determinism stress

---

## Baseline Workflows

### Capture Baseline

After making intentional DSP changes, capture new baselines:

```bash
./echoform_qa ../scenarios/echoform_critical_suite.json --capture-baseline
```

Baselines are saved to `baselines/<scenario_id>.baseline.json`

### Compare Against Baseline

Detect regressions by comparing current render to baseline:

```bash
./echoform_qa ../scenarios/echoform_critical_suite.json --compare-baseline
```

Tolerance: ±5% by default (configurable per scenario)

**Policy**: All baseline updates require PR review and justification.

---

## Scenario Authoring

### Adding a New Test

1. Create JSON scenario in `scenarios/echoform/<category>/`
2. Follow naming convention: `<feature>_<behavior>.json`
3. Add to appropriate suite (`echoform_critical_suite.json` or `echoform_comprehensive_suite.json`)
4. Capture baseline if regression tracking is needed
5. Document in `scenarios/README.md`

### Example Scenario Structure

```json
{
  "scenario_id": "my_new_test",
  "description": "Test description",
  "stimulus": {
    "type": "multitone",
    "duration_seconds": 2.0,
    "frequencies_hz": [220, 440, 880]
  },
  "dut_config": {
    "sample_rate": 48000,
    "block_size": 512,
    "num_channels": 2,
    "parameters": {
      "mix": 1.0,
      "feedback": 0.5
    }
  },
  "invariants": [
    {
      "type": "peak_level",
      "channel": "all",
      "peak_dbfs_max": -0.3
    },
    {
      "type": "silence_detection",
      "channel": "all",
      "mode": "must_not_be_silent"
    }
  ]
}
```

See [STIMULUS_REFERENCE.md](../audio-dsp-qa-harness/docs/STIMULUS_REFERENCE.md) for full options.

---

## Legacy Tests (Deprecated)

### tests/MemoryDelayEngineTests.cpp

**Status**: DEPRECATED
**Sunset Date**: 2026-03-15
**Replacement**: Harness scenarios

This file contains 2 legacy C++ unit tests that have been migrated to harness scenarios:
- `testWraparoundDsp()` → `scenarios/echoform/core/buffer_wraparound.json`
- `testCollectOverdub()` → `scenarios/echoform/feedback/collect_mode_overdub.json`

**Build**: Disabled by default (`ENABLE_TESTS=OFF`)
**CI**: Not run in CI
**Purpose**: Historical reference only
**Removal**: Scheduled for 2026-03-15 (30 days after authority cutover)

**Do not add new tests to this file.** All new DSP tests must be harness scenarios.

---

## CI/CD Integration

### Blocking Gates (DSP Quality)

**Job**: `echoform_harness_critical`
**Trigger**: Pull requests
**Suite**: `echoform_critical_suite.json`
**Blocking**: Yes
**Purpose**: Smoke test + determinism + core stability

**Job**: `echoform_harness_full`
**Trigger**: Main branch commits, nightly builds
**Suite**: `echoform_comprehensive_suite.json`
**Blocking**: Yes
**Purpose**: Full regression detection

### Non-Blocking Gates

**Job**: `echoform_harness_stress`
**Trigger**: Nightly builds
**Suite**: `echoform_stress_suite.json`
**Blocking**: No (informational)
**Purpose**: Boundary condition validation

**Job**: `echoform_harness_perf`
**Trigger**: Nightly builds
**Suite**: `echoform_performance_suite.json`
**Blocking**: No (informational)
**Purpose**: RT-safety profiling

---

## Non-DSP Testing

Currently, echoform has no non-DSP QA infrastructure (plugin format validation, UI regression, etc.).

**Policy**: If such tests are added in future, they must be clearly separated from DSP QA authority:
- ✅ Allowed: `pluginval`, UI snapshot testing, DAW compatibility checks
- ⚠️ Required: Clear labeling as "non-DSP QA" to avoid authority confusion
- ❌ Not allowed: Ad-hoc DSP validation scripts outside harness

---

## Troubleshooting

### Test Failures

**Symptom**: Scenario fails with metric out of range

**Solution**:
1. Check if this is a regression (compare against baseline)
2. Review recent DSP changes
3. If intentional, update thresholds in scenario JSON
4. If threshold change, document reason in PR

**Symptom**: Determinism test fails

**Solution**:
1. Check for non-deterministic behavior (random number generation, system time, etc.)
2. Ensure all state is reset between runs
3. Verify no external dependencies (network, filesystem)

**Symptom**: Build errors with harness

**Solution**:
1. Update harness submodule: `git submodule update --remote external/qa_harness`
2. Clean build: `rm -rf build_qa && cmake -B build_qa ...`
3. Check harness compatibility: see harness [CHANGELOG.md](../audio-dsp-qa-harness/CHANGELOG.md)

---

## Documentation

### Harness Documentation

- [QUICKSTART.md](../audio-dsp-qa-harness/docs/guides/QUICKSTART.md) — Integration guide
- [RUNTIME_GUIDE.md](../audio-dsp-qa-harness/docs/guides/RUNTIME_GUIDE.md) — Baseline workflows
- [STIMULUS_REFERENCE.md](../audio-dsp-qa-harness/docs/STIMULUS_REFERENCE.md) — Stimulus types and options
- [QA_CONTRACT.md](../audio-dsp-qa-harness/docs/QA_CONTRACT.md) — Metric tiers and thresholds
- [FAQ.md](../audio-dsp-qa-harness/docs/guides/FAQ.md) — Common questions
- [TROUBLESHOOTING.md](../audio-dsp-qa-harness/docs/guides/TROUBLESHOOTING.md) — Debug guide

### Echoform-Specific Documentation

- [ECHOFORM_QA_MIGRATION_COMPLETE.md](docs/ECHOFORM_QA_MIGRATION_COMPLETE.md) — Migration report
- [ECHOFORM_THRESHOLD_TUNING_LOG.md](docs/ECHOFORM_THRESHOLD_TUNING_LOG.md) — Threshold tuning details
- [ECHOFORM_FEATURE_COVERAGE.md](docs/ECHOFORM_FEATURE_COVERAGE.md) — Feature coverage mapping
- [scenarios/README.md](scenarios/README.md) — Scenario catalog

---

## Migration History

**Migration Start**: 2026-02-09
**Migration Complete**: 2026-02-09 (~6 hours)
**Authority Cutover**: 2026-02-13
**Legacy Sunset**: 2026-03-15 (planned)

**Key Achievements**:
- ✅ 38 scenarios created (from minimal legacy coverage)
- ✅ 15 baselines captured for regression detection
- ✅ 4 stress tests added for boundary validation
- ✅ Performance profiling scenario for RT-safety
- ✅ 100% pass rate on critical suite

**Migration Report**: [ECHOFORM_QA_MIGRATION_COMPLETE.md](docs/ECHOFORM_QA_MIGRATION_COMPLETE.md)

---

## Contributing

### Adding New DSP Features

When adding new DSP functionality:
1. **BEFORE** merging: Add harness scenario validating the new behavior
2. Add scenario to `echoform_critical_suite.json` if release-critical
3. Capture baseline if regression tracking is needed
4. Update `ECHOFORM_FEATURE_COVERAGE.md`

**This is not optional.** All DSP changes require harness scenario coverage.

### Updating DSP Behavior

When changing existing DSP:
1. Update affected scenarios' expected thresholds
2. Recapture baselines with `--capture-baseline`
3. Document rationale in PR description
4. PR review MUST validate metric changes are intentional

---

## Support

**Issues**: Open issue in [audio-dsp-qa-harness](https://github.com/joshband/audio-dsp-qa-harness/issues)
**Questions**: See harness [FAQ.md](../audio-dsp-qa-harness/docs/guides/FAQ.md)
**Plugin-Specific**: Document in `docs/` directory

---

## Authority Enforcement

**Effective Date**: 2026-02-13

**Policy Violations**:
- ❌ Adding DSP tests outside harness scenarios
- ❌ Bypassing harness in CI/CD for DSP validation
- ❌ Creating duplicate DSP baseline systems
- ❌ Treating harness as "optional" or "secondary" DSP QA

**Enforcement**:
- PR reviews MUST reject DSP changes without harness scenarios
- CI MUST block merges if critical suite fails
- Baseline updates MUST be reviewed and justified

**Exceptions**: None for DSP QA. Non-DSP testing (plugin format, UI) is explicitly allowed outside harness.

---

**Last Updated**: 2026-02-13
**Authority Status**: HARNESS-AUTHORITATIVE
**Transition Plan**: [ECHOFORM_QA_AUTHORITY_TRANSITION_PLAN.md](../audio-dsp-qa-harness/docs/migrations/ECHOFORM_QA_AUTHORITY_TRANSITION_PLAN.md)
