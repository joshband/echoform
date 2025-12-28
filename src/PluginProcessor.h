#pragma once

#include <JuceHeader.h>

class MemoryDelayEngine
{
public:
    struct Parameters
    {
        float size = 0.5f;
        float repeats = 0.35f;
        float scan = 0.0f;
        float scanMode = 0.0f;
        float spread = 0.0f;
        float routing = 0.0f;
        float collect = 0.0f;
        float always = 0.0f;
        float wipe = 0.0f;
        float inspect = 0.0f;
    };

    void prepare(double sampleRate, int maximumBlockSize);
    void setParameters(const Parameters& parameters);
    void processBlock(juce::AudioBuffer<float>& buffer);

private:
    double sampleRateHz = 44100.0;
    int maxBlockSize = 0;
    Parameters currentParameters;
};

class MemoryDelayAudioProcessor : public juce::AudioProcessor
{
public:
    MemoryDelayAudioProcessor();
    ~MemoryDelayAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    MemoryDelayEngine engine;

    std::atomic<float>* sizeParam = nullptr;
    std::atomic<float>* repeatsParam = nullptr;
    std::atomic<float>* scanParam = nullptr;
    std::atomic<float>* scanModeParam = nullptr;
    std::atomic<float>* spreadParam = nullptr;
    std::atomic<float>* routingParam = nullptr;
    std::atomic<float>* collectParam = nullptr;
    std::atomic<float>* alwaysParam = nullptr;
    std::atomic<float>* wipeParam = nullptr;
    std::atomic<float>* inspectParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemoryDelayAudioProcessor)
};
