Stereo Memory Delay Plugin – Codex‑Oriented Implementation Plan

This plan describes how to implement a stereo memory delay instrument (inspired by the Chase Bliss Habit) as an AU/VST3 plug‑in using JUCE. It assumes that you will drive code generation with an AI coding agent (e.g. OpenAI Codex) and want clear guidance on repository structure, classes, DSP logic, UI creation, and Visual DNA Studio integration. Where relevant, citations are provided from official sources to anchor the plan.

1 Create a new repository and project scaffold
1.1 Set up a local git repository and initial commit

Create a new directory for the project (e.g. stereo‑memory‑delay). Use git init to initialise the repository and add a .gitignore (at a minimum ignore build output and JUCE-generated IDE files).

Add a README.md summarising the product concept, features and UI philosophy from previous design documents. This will help future collaborators understand intent.

Create a LICENSE (e.g. MIT or GPL depending on your distribution requirements) and commit the initial files. When working with Codex or similar tools, treat these initial files as the starting point from which the agent can generate code.

1.2 Integrate JUCE as a submodule or download

Download JUCE and follow the Getting started with Projucer tutorial – JUCE’s Projucer can generate a basic audio plug‑in project and auto‑configure IDE exporters
juce.com
. The tutorial notes that the Projucer will generate code to support VST3 and AudioUnit formats
juce.com
. Alternatively, you can use JUCE’s CMake support (recommended for 2025+ projects).

Add JUCE either as a git submodule in third_party/juce or as a separate download referenced via CMake. The project will later link against JUCE modules juce_audio_basics, juce_audio_utils, juce_audio_processors, juce_gui_basics, juce_gui_extra, and juce_dsp.

1.3 Generate the initial plug‑in project

Use Projucer or CMake to generate a basic audio plug‑in. In Projucer, select Plug‑In/Basic as the project type
juce.com
; for CMake, use JUCE’s juce_add_plugin() macro. Configure the project to build VST3 and AU plug‑in formats and, if needed, a standalone tester.

Set plug‑in characteristics such as company name, version, channel layout (stereo in/out) and MIDI support using the generated CMake or Projucer project settings.

Commit the generated CMakeLists.txt or .jucer files to the repository so that Codex has a reproducible build configuration.

1.4 Set up CI/build workflow (optional at MVP)

Add a GitHub Actions workflow (e.g. .github/workflows/build.yml) to build the plug‑in on macOS and Windows runners using CMake. This ensures that the plug‑in compiles for AU and VST3 targets.

2 Design the core DSP engine

The plug‑in’s distinguishing feature is its long‑form circular memory buffer and dual playhead system. Implementation details should be encapsulated in a core DSP engine independent of JUCE’s UI layer.

2.1 Classes and responsibilities
Class	Responsibility
MemoryBuffer	Owns a circular buffer of audio samples for each channel (left/right). It should allow writing incoming audio and reading arbitrary positions relative to the current write position.
Playhead	Represents a read head that scans through the memory buffer. Each playhead maintains a time offset, scan mode (manual/automatic), and optional stereo offset. The playhead reads from the buffer, applies interpolation (linear or cubic), and passes samples through modifier chains.
ModifierChain	Holds a sequence of DSP modifiers (filters, pitch shifters, wow & flutter, dropouts etc.). Each modifier implements processSample() or processBlock() and can accumulate effects based on feedback.
MemoryDelayEngine	Manages two playheads, feedback routing, mix parameters, and deterministic random number generation. It exposes parameters for time, scan depth, spread (offset between playheads), feedback, mix and character (modifier intensity).
RandomGenerator	Implements seeded random number generation. The same seed and host position should produce identical modulation patterns across renders; this is crucial for offline render reproducibility.
2.2 Memory buffer implementation

Length – Use a configurable buffer length (e.g. default 60 seconds, adjustable via a parameter). Allocate a juce::AudioBuffer<float> with numChannels=2 and numSamples=sampleRate × maxSeconds. Pre‑allocate in the constructor to avoid heap allocation in the audio thread.

Write pointer – Maintain a sample index pointing to the current write location. Increment it each audio block and wrap around the buffer when reaching the end. Write incoming audio into the buffer (applying dry/wet mixing later in the engine).

Read access – Provide read(float positionInSamples, int channel) to return an interpolated sample at an arbitrary offset. positionInSamples is measured relative to the circular buffer’s base; convert relative offsets to absolute indices with modulo arithmetic. Linear interpolation is sufficient; for higher fidelity, implement a cubic or Lagrange interpolator.

2.3 Playhead behaviour

Manual mode – When manual scanning is enabled, the playhead’s offset is derived from a parameter (0 = no delay; 1 = maximum delay). Convert this to a sample offset into the memory buffer.

Automatic mode (wander) – When automatic scanning is enabled, the playhead periodically changes its offset according to a tempo‑synchronised period or a seeded random value. Use the RandomGenerator to select new offsets deterministically; optionally apply smoothing to avoid abrupt jumps.

Spread – For the secondary playhead, add an extra offset (parameterised) relative to the primary playhead. Spread controls the temporal distance between playheads.

Stereo options – Support separate playheads per channel (true stereo) or cross‑channel reading (feed left into right). Provide a parameter to choose between independent, mirrored or cross‑channel reading.

2.4 Feedback routing and accumulation

Feedback – After processing, mix the playheads’ output back into the memory buffer according to the feedback amount. Use a stable feedback algorithm to avoid runaway gain (e.g. limit the feedback to < 1.0, apply a slight low‑pass filter to avoid high‑frequency build‑up).

Collect vs Feed – Implement modes where only new input audio writes to the buffer, or where the plug‑in’s output (including modifiers) is fed back (closed loop). Expose this as a parameter (Collect vs Feed mode). Use a flag in MemoryDelayEngine to determine whether to include the effect output when writing into memory.

Modifier accumulation – Decide whether modifiers accumulate across feedback iterations (accumulative mode) or remain static (static mode). For example, tape‑style wow/flutter accumulates; digital filters may not.

2.5 Deterministic randomness

Implement a small RandomGenerator class that wraps a deterministic algorithm such as XorShift, seeded by:

A user‑controllable seed parameter.

The current host transport position (bars/beats or sample count), so that offline renders produce identical results.

Use this generator for automatic scan target selection, dropouts, and other randomised modifiers.

3 Expose plug‑in parameters via AudioProcessorValueTreeState

Use JUCE’s AudioProcessorValueTreeState (APVTS) to manage plug‑in parameters. Each parameter should have an ID, name, range, default value, and skew factor if necessary. Expose at least:

Parameter ID	Description	Notes
mix	Dry/wet mix (0 = dry, 1 = wet)	Provide a dry kill boolean to disable dry signal completely for send/return usage.
time	Maximum delay length (seconds or tempo‑synced note values)	Use host BPM for tempo sync.
scan	Manual scan depth (0 = now, 1 = max length)	Only active in manual mode.
autoScanRate	Automatic scan period (Hz or note values)	Controls how often automatic scanning selects a new offset.
spread	Additional offset between playheads	Can be positive or negative for left/right swapping.
feedback	Feedback amount (0–1)	Limit to < 0.99 for stability.
character	Macro controlling modifier intensity	Map to internal modifier parameters (filters, pitch drift, wow/flutter).
mode	Collect / Feed / Closed‑loop	Determines whether effect output feeds memory.
stereoMode	Independent / Linked / Cross	Chooses stereo memory behaviour.
randomSeed	Seed for deterministic randomness	Changing this will re‑seed wander and dropout patterns.

Bind parameters to UI controls in PluginEditor via SliderAttachment and ComboBoxAttachment.

4 Build the JUCE UI using Visual DNA Studio design tokens
4.1 Export design tokens from Visual DNA Studio

In Visual DNA Studio, choose a mood board or theme that matches the product (e.g. dark, cinematic, “memory, drift, erosion”). Export a Design Token JSON conforming to the W3C Design Tokens CG specification v2025.10, which defines colours, typography, spacing, radii, and motion
w3.org
. The specification emphasises theming and cross‑platform consistency
w3.org
.

Save the exported token file (e.g. visualdna_tokens.json) in the repository under resources/. This file is not compiled into the plug‑in; it is parsed at runtime to configure the UI.

4.2 Create a LookAndFeel subclass to map tokens

Implement a class TokenLookAndFeel deriving from juce::LookAndFeel_V4. In the constructor, load and parse visualdna_tokens.json. The design token spec provides names for colours, fonts, spacing, etc. Use JUCE’s Colour and Font classes to convert tokens into runtime objects.

Override methods such as getSliderThumbColour, getSliderTrackColour, getToggleButtonTextColour, getPopupMenuBackgroundColour, etc., returning colours from the tokens. For typography, create Font instances using tokenised sizes and families.

To map spacing and radii, define constants (or a struct) within TokenLookAndFeel that correspond to margin/padding values used in your Component layouts.

4.3 Layout the plug‑in UI (Play vs Inspect)

Top‑level component – Inherit from juce::AudioProcessorEditor. Set the look‑and‑feel instance at construction and apply setSize to define the default size (e.g. 400 × 300 pixels). Use a Component hierarchy rather than pixel‑based drawing for clarity.

Play Mode – Create a container (e.g. juce::FlexBox or juce::Grid) that holds the core knobs/sliders: mix, time, scan, spread, feedback, character. Use juce::Slider with RotaryVerticalDrag style for knobs; apply the token colours and fonts via your TokenLookAndFeel. Avoid numeric values; use labels like “Mix” or icons.

Inspect Mode – Provide a hidden panel or overlay. On a modifier key (e.g. shift‑click) or pressing a small “Inspect” button, reveal additional visualisation components:

A memory timeline component that renders a non‑literal heat map of past vs current memory. Represent energy/density rather than waveform; the design token spec’s colours and spacing inform the look. Use juce::Path or juce::Graphics to draw.

Indicators for primary and secondary playheads (e.g. small circles moving along the timeline). Ensure that Inspect mode does not alter DSP state (observational only).

Parameter attachments – Use AudioProcessorValueTreeState::SliderAttachment to bind each knob to its parameter. Provide tooltips or subtle hints (not in UI by default) that explain behaviour when the user hovers or long‑presses.

Responsiveness – Use relative layout (FlexBox/Grid) so the UI scales gracefully. Use tokenised spacing and radii to keep consistent proportions.

4.4 Load tokens at runtime

During plug‑in construction (TokenLookAndFeel), parse the token JSON. Use cross‑platform libraries (e.g. JUCE’s var JSON parser or juce::JSON::parse()). Validate token schema; fallback to default colours if tokens are missing. This decouples UI styling from code and allows Visual DNA Studio to iterate on design without touching C++ code.

5 Integrate DSP and UI

Instantiate MemoryDelayEngine in AudioProcessor (e.g. PluginProcessor). During prepareToPlay(), pass the sample rate and maximum expected block size to MemoryDelayEngine so it can allocate buffers.

In processBlock(), call MemoryDelayEngine::processBlock() passing input and output buffers and the parameter values from APVTS. The engine returns the processed audio; mix with dry signal according to the mix parameter.

Expose pointers or references to APVTS in PluginEditor to create attachments. Use the TokenLookAndFeel for all UI components.

6 Testing and validation

Use JUCE’s built‑in Audio Plugin Host to test the plug‑in. The host can load VST3 and AU plug‑ins and provides a node‑based patch view
juce.com
. Connect MIDI and audio inputs to the plug‑in, adjust parameters and verify that memory scanning, feedback and modifiers behave as intended.

Verify deterministic behaviour by bouncing a session offline multiple times with the same seed and verifying identical results. Change the seed and confirm that randomness changes accordingly.

Test extreme parameter values (e.g. maximum feedback, minimum mix) to ensure stability and that no NaNs or infinities appear in the audio buffer.

Benchmark CPU usage. Because the plug‑in may run long sessions, ensure that memory buffer operations and random modulation do not cause CPU spikes. Optimise by using cache‑friendly buffer wraps and minimising branch mispredictions.

7 Future expansion and maximal features

After shipping the MVP, consider implementing features flagged as “maximal” in the product vision:

Snapshot/Memory Freeze – Allow capturing the memory buffer and switching between multiple snapshots. Add a snapshot manager class and UI components to recall and crossfade snapshots.

Multi‑instance memory awareness – Allow two plug‑in instances to share memory via an IPC or shared memory mechanism. This is complex; design an API for instances to register with a central memory bus. Provide UI for selecting which instance to follow.

Mid/Side and spatial modes – Extend the memory engine to allow separate mid/side buffers; add parameters to scan only side channels. Provide a UI to switch between stereo, mid/side and cross modes.

Preset morphing – Implement interpolation between presets without clearing memory. Use parameter smoothing and crossfade memory states. Provide a macro knob for morph amount.

8 Summary of key references

JUCE’s Projucer can generate a basic audio plug‑in and provides project settings for plugin formats and MIDI options
juce.com
juce.com
. Exporters support multiple IDEs (Xcode, Visual Studio, Makefile, etc.)
juce.com
.

The Design Tokens Specification v2025.10 defines how colours, typography, spacing and other design decisions are represented and exchanged across tools
w3.org
. It introduces theming, modern colour support and cross‑platform consistency, making design tokens a single source of truth
w3.org
.

JUCE provides comprehensive UI capabilities; its documentation notes that JUCE handles 2D rendering, image formats and fonts, and offers various LookAndFeel customisation options for scalable UIs
juce.com
. These features make it possible to map design tokens onto the UI.

This plan gives you a clear roadmap for coding the plug‑in using Codex or any other coding assistant. By following the steps above, you will build a robust core DSP engine, design a UI driven by Visual DNA Studio tokens, and structure your repository for maintainability and collaboration.