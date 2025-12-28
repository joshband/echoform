#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto sizeId = "size";
constexpr auto repeatsId = "repeats";
constexpr auto scanId = "scan";
constexpr auto scanModeId = "scanMode";
constexpr auto spreadId = "spread";
constexpr auto routingId = "routing";
constexpr auto collectId = "collect";
constexpr auto alwaysId = "always";
constexpr auto wipeId = "wipe";
constexpr auto inspectId = "inspect";
}

void MemoryDelayEngine::prepare(double sampleRate, int maximumBlockSize)
{
    sampleRateHz = sampleRate;
    maxBlockSize = maximumBlockSize;
}

void MemoryDelayEngine::setParameters(const Parameters& parameters)
{
    currentParameters = parameters;
}

void MemoryDelayEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::ignoreUnused(buffer, sampleRateHz, maxBlockSize, currentParameters);
}

MemoryDelayAudioProcessor::MemoryDelayAudioProcessor()
    : juce::AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                               .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    sizeParam = apvts.getRawParameterValue(sizeId);
    repeatsParam = apvts.getRawParameterValue(repeatsId);
    scanParam = apvts.getRawParameterValue(scanId);
    scanModeParam = apvts.getRawParameterValue(scanModeId);
    spreadParam = apvts.getRawParameterValue(spreadId);
    routingParam = apvts.getRawParameterValue(routingId);
    collectParam = apvts.getRawParameterValue(collectId);
    alwaysParam = apvts.getRawParameterValue(alwaysId);
    wipeParam = apvts.getRawParameterValue(wipeId);
    inspectParam = apvts.getRawParameterValue(inspectId);
}

MemoryDelayAudioProcessor::~MemoryDelayAudioProcessor() = default;

const juce::String MemoryDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MemoryDelayAudioProcessor::acceptsMidi() const
{
    return false;
}

bool MemoryDelayAudioProcessor::producesMidi() const
{
    return false;
}

bool MemoryDelayAudioProcessor::isMidiEffect() const
{
    return false;
}

double MemoryDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MemoryDelayAudioProcessor::getNumPrograms()
{
    return 1;
}

int MemoryDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MemoryDelayAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String MemoryDelayAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void MemoryDelayAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void MemoryDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
}

void MemoryDelayAudioProcessor::releaseResources()
{
}

bool MemoryDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void MemoryDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    MemoryDelayEngine::Parameters parameters;
    parameters.size = sizeParam->load();
    parameters.repeats = repeatsParam->load();
    parameters.scan = scanParam->load();
    parameters.scanMode = scanModeParam->load();
    parameters.spread = spreadParam->load();
    parameters.routing = routingParam->load();
    parameters.collect = collectParam->load();
    parameters.always = alwaysParam->load();
    parameters.wipe = wipeParam->load();
    parameters.inspect = inspectParam->load();

    engine.setParameters(parameters);
    engine.processBlock(buffer);
}

bool MemoryDelayAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MemoryDelayAudioProcessor::createEditor()
{
    return new MemoryDelayAudioProcessorEditor(*this);
}

void MemoryDelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void MemoryDelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MemoryDelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(sizeId, "Size",
                                                                 juce::NormalisableRange<float>(0.0f, 1.0f),
                                                                 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(repeatsId, "Repeats",
                                                                 juce::NormalisableRange<float>(0.0f, 1.0f),
                                                                 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(scanId, "Scan",
                                                                 juce::NormalisableRange<float>(0.0f, 1.0f),
                                                                 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(scanModeId, "Scan Mode",
                                                                  juce::StringArray{"Forward", "PingPong", "Random"},
                                                                  0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(spreadId, "Spread",
                                                                 juce::NormalisableRange<float>(0.0f, 1.0f),
                                                                 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(routingId, "Routing",
                                                                  juce::StringArray{"Stereo", "Swap", "Mid/Side"},
                                                                  0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(collectId, "Collect", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>(alwaysId, "Always", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(wipeId, "Wipe", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(inspectId, "Inspect", false));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MemoryDelayAudioProcessor();
}
