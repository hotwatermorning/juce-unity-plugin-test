#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace ParamID
{
    inline juce::String kGain = "gain";
    inline juce::String kReverbLevel = "reverb";
    inline juce::String kPlayKeyC = "key-c";
    inline juce::String kPlayKeyD = "key-d";
    inline juce::String kPlayKeyE = "key-e";
    inline juce::String kPlayKeyF = "key-f";
    inline juce::String kPlayKeyG = "key-g";
    inline juce::String kPlayKeyA = "key-a";
    inline juce::String kPlayKeyB = "key-b";
}

std::vector<juce::String> const & getPlayKeyIDs();

namespace AppDefines
{
    inline float gainDecibelMin = -48.0f;
    inline float gainDecibelMax = 6.0f;
    inline float gainDecibelAudibelLimit = -47.9f;
}

//==============================================================================
class AudioPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    struct Synthesizer;
    using SynthesizerPtr = std::unique_ptr<Synthesizer>;

    std::map<juce::String, SynthesizerPtr> synthesizers;
    float lastGain = 0.0;

    juce::dsp::Reverb reverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
