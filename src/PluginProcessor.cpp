#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <memory>
#include <vector>
#include "MemoryDelayEngine.h"

namespace {
constexpr float kBufferSeconds = 180.0f;
// Free function to create the parameter layout.  This uses
// std::make_unique to create AudioParameter instances, which is the
// recommended pattern for JUCE 6+.  See JUCE forum discussion on
// using ParameterLayout with AudioProcessorValueTreeState【670410746428985†L124-L134】.
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    // Mix: 0 = dry, 1 = wet
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f));
    // Time: 0..30 seconds window for the tape playback head
    params.push_back(std::make_unique<juce::AudioParameterFloat>("time", "Time", 0.0f, 30.0f, 3.0f));
    // Host bypass parameter
    params.push_back(std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false));
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
juce::AudioProcessorParameter* StereoMemoryDelayAudioProcessor::getBypassParameter() const
{
    return parameters.getParameter("bypass");
}

int StereoMemoryDelayAudioProcessor::getNumPrograms() { return 1; }
int StereoMemoryDelayAudioProcessor::getCurrentProgram() { return 0; }
void StereoMemoryDelayAudioProcessor::setCurrentProgram (int) {}
const juce::String StereoMemoryDelayAudioProcessor::getProgramName (int) { return {}; }
void StereoMemoryDelayAudioProcessor::changeProgramName (int, const juce::String&) {}

void StereoMemoryDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Allocate DSP engine and prepare resources
    engine->prepare(sampleRate, samplesPerBlock, kBufferSeconds);
    // Set initial parameter values
    engine->setMix(*parameters.getRawParameterValue("mix"));
    engine->setTapeMode(true);
    engine->setTapeWindowSeconds(*parameters.getRawParameterValue("time"));
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
    engine->setTapeWindowSeconds(*parameters.getRawParameterValue("time"));
    bool hostBypassed = false;
    if (auto* bypassParam = getBypassParameter())
        hostBypassed = bypassParam->getValue() > 0.5f;
    engine->setBypassed(hostBypassed);

    int64_t transportSamples = -1;
    bool isPlaying = false;
    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            isPlaying = position->getIsPlaying();
            if (auto timeInSamples = position->getTimeInSamples())
            {
                transportSamples = *timeInSamples;
            }
            else if (auto timeInSeconds = position->getTimeInSeconds())
            {
                transportSamples = static_cast<int64_t>(*timeInSeconds * getSampleRate());
            }
            else if (auto ppqPosition = position->getPpqPosition())
            {
                if (auto bpm = position->getBpm())
                {
                    const double seconds = (*ppqPosition) * (60.0 / *bpm);
                    transportSamples = static_cast<int64_t>(seconds * getSampleRate());
                }
            }
        }
    }
    engine->setTransportPosition(transportSamples, isPlaying);
    // Process audio
    engine->processBlock(buffer);
}

bool StereoMemoryDelayAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* StereoMemoryDelayAudioProcessor::createEditor()
{
#if ECHOFORM_USE_VDNA_UI
    return new VisualDNAEditor(*this);
#else
    return new StereoMemoryDelayAudioProcessorEditor(*this);
#endif
}

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
