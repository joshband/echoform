#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <memory>
#include <vector>
#include "MemoryDelayEngine.h"

namespace {
constexpr float kMaxDelaySeconds = 10.0f;
// Free function to create the parameter layout.  This uses
// std::make_unique to create AudioParameter instances, which is the
// recommended pattern for JUCE 6+.  See JUCE forum discussion on
// using ParameterLayout with AudioProcessorValueTreeState【670410746428985†L124-L134】.
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    // Mix: 0 = dry, 1 = wet
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f));
    // Scan: manual delay position 0..1
    params.push_back(std::make_unique<juce::AudioParameterFloat>("scan", "Scan", 0.0f, 1.0f, 0.0f));
    // Auto Scan Rate: 0 = manual, otherwise rate in Hz
    params.push_back(std::make_unique<juce::AudioParameterFloat>("autoScanRate", "Auto Scan Rate", 0.0f, 2.0f, 0.0f));
    // Spread: additional offset between playheads in seconds (-2..2)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("spread", "Spread", -2.0f, 2.0f, 0.0f));
    // Feedback: 0..0.99
    params.push_back(std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.99f, 0.0f));
    // Time: maximum delay length in seconds (0.1..10)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("time", "Time", 0.1f, kMaxDelaySeconds, 1.0f));
    // Character: macro controlling modifier intensity
    params.push_back(std::make_unique<juce::AudioParameterFloat>("character", "Character", 0.0f, 1.0f, 0.0f));
    // Stereo Mode: Independent / Linked / Cross
    params.push_back(std::make_unique<juce::AudioParameterChoice>("stereoMode", "Stereo Mode",
        juce::StringArray { "Independent", "Linked", "Cross" }, 0));
    // Feedback Mode: Collect / Feed / Closed
    params.push_back(std::make_unique<juce::AudioParameterChoice>("mode", "Mode",
        juce::StringArray { "Collect", "Feed", "Closed" }, 1));
    // Random seed for deterministic modulation
    params.push_back(std::make_unique<juce::AudioParameterInt>("randomSeed", "Random Seed", 0, 65535, 1));
    return { params.begin(), params.end() };
}
} // namespace

StereoMemoryDelayAudioProcessor::StereoMemoryDelayAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters (*this, nullptr, juce::Identifier("PARAMS"), createParameterLayout())
{
    // create DSP engine
    engine = std::make_unique<MemoryDelayEngine>();
}

StereoMemoryDelayAudioProcessor::~StereoMemoryDelayAudioProcessor() {}

const juce::String StereoMemoryDelayAudioProcessor::getName() const { return JucePlugin_Name; }

bool StereoMemoryDelayAudioProcessor::acceptsMidi() const { return false; }
bool StereoMemoryDelayAudioProcessor::producesMidi() const { return false; }
bool StereoMemoryDelayAudioProcessor::isMidiEffect() const { return false; }
double StereoMemoryDelayAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int StereoMemoryDelayAudioProcessor::getNumPrograms() { return 1; }
int StereoMemoryDelayAudioProcessor::getCurrentProgram() { return 0; }
void StereoMemoryDelayAudioProcessor::setCurrentProgram (int) {}
const juce::String StereoMemoryDelayAudioProcessor::getProgramName (int) { return {}; }
void StereoMemoryDelayAudioProcessor::changeProgramName (int, const juce::String&) {}

void StereoMemoryDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Allocate DSP engine and prepare resources
    engine->prepare(sampleRate, samplesPerBlock, kMaxDelaySeconds);
    // Set initial parameter values
    engine->setMix(*parameters.getRawParameterValue("mix"));
    engine->setScan(*parameters.getRawParameterValue("scan"));
    engine->setAutoScanRate(*parameters.getRawParameterValue("autoScanRate"));
    engine->setSpread(*parameters.getRawParameterValue("spread"));
    engine->setFeedback(*parameters.getRawParameterValue("feedback"));
    engine->setTime(*parameters.getRawParameterValue("time"));
    engine->setCharacter(*parameters.getRawParameterValue("character"));
    engine->setStereoMode(static_cast<int>(*parameters.getRawParameterValue("stereoMode")));
    engine->setMode(static_cast<int>(*parameters.getRawParameterValue("mode")));
    engine->setRandomSeed(static_cast<int>(*parameters.getRawParameterValue("randomSeed")));
}

void StereoMemoryDelayAudioProcessor::releaseResources()
{
    // Release resources
    engine.reset();
    engine = std::make_unique<MemoryDelayEngine>();
}

bool StereoMemoryDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // Only stereo in/out supported
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
#endif
}

void StereoMemoryDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels not used
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Update engine parameters from APVTS
    engine->setMix(*parameters.getRawParameterValue("mix"));
    engine->setScan(*parameters.getRawParameterValue("scan"));
    engine->setAutoScanRate(*parameters.getRawParameterValue("autoScanRate"));
    engine->setSpread(*parameters.getRawParameterValue("spread"));
    engine->setFeedback(*parameters.getRawParameterValue("feedback"));
    engine->setTime(*parameters.getRawParameterValue("time"));
    engine->setCharacter(*parameters.getRawParameterValue("character"));
    engine->setStereoMode(static_cast<int>(*parameters.getRawParameterValue("stereoMode")));
    engine->setMode(static_cast<int>(*parameters.getRawParameterValue("mode")));
    engine->setRandomSeed(static_cast<int>(*parameters.getRawParameterValue("randomSeed")));

    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    if (auto* playHead = getPlayHead())
        playHead->getCurrentPosition(posInfo);

    int64_t transportSamples = -1;
    if (posInfo.timeInSamples > 0)
        transportSamples = posInfo.timeInSamples;
    else if (posInfo.ppqPosition > 0.0 && posInfo.bpm > 0.0)
    {
        const double seconds = posInfo.ppqPosition * (60.0 / posInfo.bpm);
        transportSamples = static_cast<int64_t>(seconds * getSampleRate());
    }
    engine->setTransportPosition(transportSamples, posInfo.isPlaying);
    // Process audio
    engine->processBlock(buffer);
}

bool StereoMemoryDelayAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* StereoMemoryDelayAudioProcessor::createEditor() { return new StereoMemoryDelayAudioProcessorEditor (*this); }

void StereoMemoryDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // TODO: store state using parameters
    juce::MemoryOutputStream stream(destData, true);
    parameters.state.writeToStream(stream);
}

void StereoMemoryDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // TODO: restore state using parameters
    juce::ValueTree tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
        parameters.state = tree;
}

void StereoMemoryDelayAudioProcessor::getVisualSnapshot(MemoryDelayEngine::VisualSnapshot& snapshot) const
{
    if (engine != nullptr)
        engine->getVisualSnapshot(snapshot);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoMemoryDelayAudioProcessor();
}
