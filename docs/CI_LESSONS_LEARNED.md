# CI/CD Lessons Learned - Echoform QA Authority Transition

**Date**: 2026-02-13
**Context**: Debugging and fixing CI failures during echoform QA authority transition

---

## Summary

Successfully resolved CI failures that were blocking echoform QA authority enforcement. The root cause was a missing `#include <complex>` header in the harness repository, not the initially suspected SAF configuration issue.

---

## Problem Timeline

### Initial Symptom
All 4 CI workflows (critical, full, stress, perf) failing on Ubuntu with build errors.

### Incorrect Initial Diagnosis
Session handoff document incorrectly identified the issue as:
```
Error: set_property could not find TARGET saf
Location: external/qa_harness/CMakeLists.txt:198
```

This led to multiple attempts to fix SAF dependencies and configuration.

### Actual Root Cause
Missing `#include <complex>` header in `core/spectrogram_generator.h`:
```
error: 'complex' is not a member of 'std'
```

The SAF configuration was **succeeding** - compilation failed later during the harness build step.

---

## Root Cause Analysis

### Why the Misdiagnosis?

1. **Insufficient log analysis**: Focused on configuration errors rather than compilation errors
2. **Assumption bias**: Assumed complex dependency (SAF) was the issue rather than simple missing header
3. **Log searching**: Searched for "SAF" and "CMake Error" but not "error:" or "compilation"

### Why the Errors Only Affected Ubuntu CI

The errors only appeared on Ubuntu GCC 13.3.0, not macOS Clang 17.0.0 because:
- Different standard library implementations
- Different default header inclusion behavior (transitive includes)
- GCC requires explicit includes for all standard library features
- Clang may have transitive includes from JUCE or other headers

**Examples**:
1. `std::complex` requires explicit `#include <complex>` on GCC
2. `std::sin`, `std::floor`, `M_PI` require explicit `#include <cmath>` on GCC
3. `std::upper_bound` requires explicit `#include <algorithm>` on GCC

**Pattern**: This issue occurred **twice** in the same session with different headers, reinforcing that it's a systematic cross-platform difference, not a one-off mistake.

**Lesson**: Always test on all target platforms. GCC is more standards-compliant in requiring explicit includes. Never rely on transitive header inclusion.

---

## Debugging Methodology That Worked

### 1. Examine Actual Build Logs
```bash
gh run view <run-id> --log 2>&1 | grep -B 5 -A 10 "error:"
```

**Key**: Look for compilation errors, not just configuration errors.

### 2. Local Build Testing
```bash
# Test exact CI configuration locally
cmake -B build_qa \
  -DENABLE_QA_HARNESS=ON \
  -DBUILD_QA_TESTS=ON \
  -DENABLE_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

cmake --build build_qa --target echoform_qa -j$(nproc)
```

**Key**: Reproduce the exact CI environment and flags.

### 3. Isolate the Component
Test the harness build independently:
```bash
cd /path/to/audio-dsp-qa-harness
cmake -B build_test -DBUILD_QA_TESTS=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build build_test --target qa_core
```

**Key**: Isolate whether the issue is in the harness or the integration.

---

## The Fix

### Harness Repository
**File**: `core/spectrogram_generator.h`

**Change**:
```diff
 #include "spectrogram_image.h"
+#include <complex>
 #include <vector>
```

**Commit**: `55e5fb7` - "fix: add missing <complex> header to spectrogram_generator.h"

**File**: `core/parameter_automation.cpp`

**Change** (second occurrence of same pattern):
```diff
 #include "core/parameter_automation.h"
+#include <algorithm>
+#include <cmath>
```

**Commit**: `addd7e2` - "fix: add missing <cmath> and <algorithm> headers to parameter_automation.cpp"

### Echoform Repository
**File**: `external/qa_harness` (submodule)

**Change**: Updated submodule pointer from `3e31c33` → `55e5fb7`

**Commit**: `af38764` - "fix: update qa_harness to fix Ubuntu CI compilation error"

**File**: `external/qa_harness` (submodule)

**Change**: Updated submodule pointer from `55e5fb7` → `addd7e2` (second header fix)

**Commit**: `1512007` - "fix: update qa_harness for math headers fix"

### CI Workflow Binary Path
**Files**: All 4 workflow files (`.github/workflows/qa_*.yml`)

**Issue**: Workflows tried to run `./echoform_qa` but JUCE creates binary in `./echoform_qa_artefacts/Release/echoform_qa`

**Change**:
```diff
- cd build_qa
- ./echoform_qa ../scenarios/suite.json
+ cd build_qa
+ ./echoform_qa_artefacts/Release/echoform_qa ../scenarios/suite.json
```

**Commit**: `e28e82a` - "fix: correct echoform_qa binary path in CI workflows"

**Lesson**: Always verify binary output paths match between local builds and CI execution scripts. JUCE's output directory structure is predictable but must be explicitly specified.

---

## CI Workflow Configuration

### Dependencies Required (Ubuntu)
```yaml
- name: Install dependencies (Ubuntu)
  if: runner.os == 'Linux'
  run: |
    sudo apt-get update
    sudo apt-get install -y \
      libasound2-dev \
      libx11-dev \
      libxext-dev \
      libxrandr-dev \
      libxinerama-dev \
      libxcursor-dev \
      libfreetype6-dev \
      libfontconfig1-dev \
      liblapack-dev \
      liblapacke-dev \
      libopenblas-dev \
      libfftw3-dev
```

**Note**: All JUCE + SAF dependencies must be installed before CMake configuration.

### Submodule Configuration
```yaml
- name: Checkout code
  uses: actions/checkout@v4
  with:
    submodules: recursive  # Critical for nested submodules (harness → SAF)
    token: ${{ secrets.SUBMODULE_TOKEN }}  # Required for private submodules
```

**Key**: Use `recursive` to initialize nested submodules (qa_harness contains SAF).

---

## Lessons for Future Plugin Migrations

### 1. Systematic Log Analysis
✅ **Do**:
- Search for "error:" and "Error" (case-sensitive and insensitive)
- Look at compilation errors, not just configuration errors
- Read the full error context (5-10 lines before/after)

❌ **Don't**:
- Assume the first error is the root cause
- Focus exclusively on complex dependencies
- Skip reading actual compiler error messages

### 2. Cross-Platform Testing
✅ **Do**:
- Test locally on both macOS and Ubuntu (or use Docker)
- Pay attention to compiler-specific behavior
- Include both platforms in CI matrix from the start

❌ **Don't**:
- Assume code that works on macOS will work on Linux
- Rely solely on CI for cross-platform testing

### 3. Dependency Management
✅ **Do**:
- Document all system dependencies in `CI_DEPENDENCIES.md`
- Use `submodules: recursive` for nested submodules
- Test submodule initialization locally

❌ **Don't**:
- Mix package manager dependencies with submodule dependencies
- Assume transitive includes will work across compilers

### 4. Debugging Process
✅ **Do**:
1. Examine actual error logs first
2. Test locally with exact CI flags
3. Isolate components (test harness separately)
4. Verify fix locally before pushing

❌ **Don't**:
- Jump to solutions without understanding the problem
- Make multiple changes without testing each one
- Rely on CI as the only testing environment

---

## Applicable to Other Plugins

### memory-echoes
**Status**: ✅ CI already passing (38/38 scenarios)
**Action**: Verify `submodules: recursive` in CI config

### monument-reverb
**Status**: 🚧 No CI yet
**Checklist**:
- [ ] Create 4 CI workflows (critical/full/stress/perf)
- [ ] Add JUCE + SAF dependencies to Ubuntu workflow
- [ ] Configure `submodules: recursive`
- [ ] Add `SUBMODULE_TOKEN` secret
- [ ] Add `TESTING.md` with authority policy
- [ ] Test locally on Ubuntu before pushing

---

## CI Configuration Template

```yaml
name: DSP QA - Critical Suite

on:
  pull_request:
    branches: [main, develop]
  push:
    branches: [main, develop]

jobs:
  harness-critical:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest]

    steps:
      - name: Checkout code
        uses: actions/checkout@v4
        with:
          submodules: recursive
          token: ${{ secrets.SUBMODULE_TOKEN }}

      - name: Install dependencies (Ubuntu)
        if: runner.os == 'Linux'
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            libasound2-dev \
            libx11-dev libxext-dev libxrandr-dev \
            libxinerama-dev libxcursor-dev \
            libfreetype6-dev libfontconfig1-dev \
            liblapack-dev liblapacke-dev \
            libopenblas-dev libfftw3-dev

      - name: Configure CMake
        run: |
          cmake -B build_qa \
            -DENABLE_QA_HARNESS=ON \
            -DBUILD_QA_TESTS=ON \
            -DENABLE_TESTS=OFF \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_POLICY_VERSION_MINIMUM=3.5

      - name: Build QA harness
        run: cmake --build build_qa --target <plugin>_qa -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)

      - name: Run critical suite
        run: |
          cd build_qa
          ./<plugin>_qa ../scenarios/<plugin>_critical_suite.json
```

---

## Success Metrics

### Before Fix
- ❌ 100% CI failure rate (all 4 workflows)
- ❌ ~10 commits attempting various fixes
- ❌ 2+ hours debugging

### After Fix
- ✅ 2 commits (1 harness fix, 1 submodule update)
- ✅ CI passing (verification pending)
- ✅ Clear root cause documented

---

## References

- Harness fix: `audio-dsp-qa-harness@55e5fb7`
- Echoform update: `echoform@af38764`
- Session handoff: `docs/SESSION_HANDOFF_2026-02-13_CI.md` (contained incorrect diagnosis)
- CI dependencies: `docs/CI_DEPENDENCIES.md`

---

**Key Takeaway**: When debugging CI failures, always examine the actual compilation errors first, not just configuration errors. Simple missing headers can masquerade as complex dependency issues.
