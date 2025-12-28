#include "PluginProcessor.h"
#include "PluginEditor.h"

StereoMemoryDelayAudioProcessor::StereoMemoryDelayAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, juce::Identifier("StereoMemoryDelay"), createParameterLayout())
{
}

const juce::String StereoMemoryDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StereoMemoryDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StereoMemoryDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StereoMemoryDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StereoMemoryDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StereoMemoryDelayAudioProcessor::getNumPrograms()
{
    return 1;
}

int StereoMemoryDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StereoMemoryDelayAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String StereoMemoryDelayAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void StereoMemoryDelayAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void StereoMemoryDelayAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sampleRateHz = sampleRate;
    engine.prepare(sampleRate, samplesPerBlock);
}

void StereoMemoryDelayAudioProcessor::releaseResources()
{
}

#if !JucePlugin_PreferredChannelConfigurations
bool StereoMemoryDelayAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void StereoMemoryDelayAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    updateTransportPosition();

    const auto seedValue = parameters.getRawParameterValue("randomSeed");
    if (seedValue != nullptr)
    {
        const double seed = static_cast<double>(seedValue->load());
        randomSeedValue.store(seed);
        engine.setRandomSeed(seed, transportPositionSamples.load());
    }

    engine.processBlock(buffer);
}

bool StereoMemoryDelayAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* StereoMemoryDelayAudioProcessor::createEditor()
{
    return new StereoMemoryDelayAudioProcessorEditor(*this);
}

void StereoMemoryDelayAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StereoMemoryDelayAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

juce::AudioProcessorValueTreeState& StereoMemoryDelayAudioProcessor::getValueTreeState() noexcept
{
    return parameters;
}

double StereoMemoryDelayAudioProcessor::getTransportPositionSamples() const noexcept
{
    return transportPositionSamples.load();
}

double StereoMemoryDelayAudioProcessor::getRandomSeedValue() const noexcept
{
    return randomSeedValue.load();
}

void StereoMemoryDelayAudioProcessor::updateTransportPosition()
{
    double positionSamples = 0.0;
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (auto timeInSamples = positionInfo->getTimeInSamples())
                positionSamples = static_cast<double>(*timeInSamples);
            else if (auto timeInSeconds = positionInfo->getTimeInSeconds())
                positionSamples = static_cast<double>(*timeInSeconds) * sampleRateHz;
            else if (auto ppqPosition = positionInfo->getPpqPosition())
            {
                if (auto bpm = positionInfo->getBpm())
                {
                    const double seconds = (static_cast<double>(*ppqPosition) * 60.0) / static_cast<double>(*bpm);
                    positionSamples = seconds * sampleRateHz;
                }
            }
        }
    }

    transportPositionSamples.store(positionSamples);
}

juce::AudioProcessorValueTreeState::ParameterLayout StereoMemoryDelayAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("randomSeed", 1),
        "Random Seed",
        juce::NormalisableRange<float>(0.0f, 100000.0f, 1.0f),
        0.0f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StereoMemoryDelayAudioProcessor();
}
