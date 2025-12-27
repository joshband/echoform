#include "PluginProcessor.h"
#include "PluginEditor.h"

StereoMemoryDelayAudioProcessor::StereoMemoryDelayAudioProcessor()
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters (*this, nullptr)
{
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
    // TODO: allocate DSP engine and prepare resources
}

void StereoMemoryDelayAudioProcessor::releaseResources()
{
    // TODO: release resources and memory
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels not used
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // TODO: process memory delay engine
}

bool StereoMemoryDelayAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* StereoMemoryDelayAudioProcessor::createEditor() { return new juce::GenericAudioProcessorEditor (*this); }

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
