#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <BinaryData.h>

std::vector<juce::String> const & getPlayKeyIDs()
{
    static std::vector<juce::String> const IDs {
        ParamID::kPlayKeyC,
        ParamID::kPlayKeyD,
        ParamID::kPlayKeyE,
        ParamID::kPlayKeyF,
        ParamID::kPlayKeyG,
        ParamID::kPlayKeyA,
        ParamID::kPlayKeyB,
    };

    return IDs;
}

struct AudioPluginAudioProcessor::Synthesizer
{
    void play()
    {
        synth.noteOn(0, 60, 1.0);
        playing = true;
    }

    void stop()
    {
        synth.noteOff(0, 60, 1.0, true);
        playing = false;
    }

    bool isPlaying() const
    {
        return playing;
    }

    void setSample(char const *data, int size)
    {
        juce::MemoryBlock tmp(data, size);
        sampleData.swapWith(tmp);
    }

    void prepareToPlay(double sampleRate, int blockSize)
    {
        assert(sampleData.getSize() > 0);

        juce::WavAudioFormat fmt;

        std::unique_ptr<juce::AudioFormatReader> reader(
            fmt.createReaderFor(new juce::MemoryInputStream(sampleData, false), false)
            );

        double attackTime = 0.05;
        double releaseTime = 0.1;

        juce::BigInteger noteRange;
        noteRange.setRange(0, 127, true);

        synth.clearSounds();
        synth.addSound(new juce::SamplerSound("default",
                                               *reader,
                                               noteRange,
                                               60,
                                               attackTime,
                                               releaseTime,
                                               6.0));
        synth.setCurrentPlaybackSampleRate(sampleRate);

        for(int i = 0; i < 10; ++i) {
            synth.addVoice(new juce::SamplerVoice());
        }
        playing = false;
    }

    void processBlock(juce::AudioSampleBuffer &buf)
    {
        synth.renderNextBlock(buf, midiBuffer, 0, buf.getNumSamples());
    }

    juce::MemoryBlock sampleData;
    juce::Synthesiser synth;
    juce::MidiBuffer midiBuffer;
    bool playing = false;
};

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
, apvts(*this, nullptr, "AudioProcessorState", createParameterLayout())
{
    for(auto const &key: getPlayKeyIDs()) {
        synthesizers[key] = std::make_unique<Synthesizer>();
    }

#define SET_SAMPLE(KeyName) \
    synthesizers[ParamID::kPlayKey ## KeyName]->setSample(BinaryData::KeyName ## _wav, BinaryData::KeyName ## _wavSize);

    SET_SAMPLE(C);
    SET_SAMPLE(D);
    SET_SAMPLE(E);
    SET_SAMPLE(F);
    SET_SAMPLE(G);
    SET_SAMPLE(A);
    SET_SAMPLE(B);
#undef SET_SAMPLE
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    for(auto &entry: synthesizers) {
        entry.second->prepareToPlay(sampleRate, samplesPerBlock);
    }

    lastGain = 0;

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = std::max<int>(getTotalNumInputChannels(), getTotalNumOutputChannels());
    spec.sampleRate = sampleRate;
    reverb.prepare(spec);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto const &paramIDs = getPlayKeyIDs();
    for(auto const &paramID: paramIDs) {
        auto param = static_cast<juce::AudioParameterBool *>(apvts.getParameter(paramID));
        bool isOn = param->get();

        auto &synth = synthesizers[paramID];
        if(synth->isPlaying() == false && isOn) {
            synth->play();
        }
        if(synth->isPlaying() && isOn == false) {
            synth->stop();
        }
    }

    for(auto &entry: synthesizers) {
        entry.second->processBlock(buffer);
    }

    float gain = juce::Decibels::decibelsToGain(
        dynamic_cast<juce::AudioParameterFloat&>(*apvts.getParameter(ParamID::kGain)).get(),
        AppDefines::gainDecibelAudibelLimit
    );

    int reverbLevel = dynamic_cast<juce::AudioParameterInt&>(*apvts.getParameter(ParamID::kReverbLevel)).get();
    juce::dsp::Reverb::Parameters reverbParam;
    reverbParam.damping = 0.7;
    reverbParam.wetLevel = reverbLevel / 100.0;
    reverbParam.dryLevel = 1.0 - reverbParam.wetLevel;
    reverbParam.roomSize = 0.7;
    reverbParam.width = 0.6;
    reverb.setParameters(reverbParam);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);

    buffer.applyGainRamp(0, buffer.getNumSamples(), lastGain, gain);
    lastGain = gain;

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
        // ..do something to the data...
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    // return new AudioPluginAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        ParamID::kGain,
        ParamID::kGain,
        juce::NormalisableRange<float>{AppDefines::gainDecibelMin, AppDefines::gainDecibelMax},
        0.0f,
        "",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int /*maxLength*/) {
            if(value < AppDefines::gainDecibelAudibelLimit)
            {
                return juce::String("-inf");
            }
            return juce::String::formatted("%0.1f", value);
        },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        ParamID::kReverbLevel,
        ParamID::kReverbLevel,
        0, 100,
        0,
        " %"
        ));

    for(auto keyID: getPlayKeyIDs()) {
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            keyID,
            keyID,
            false
            ));
    }

    return juce::AudioProcessorValueTreeState::ParameterLayout(params.begin(), params.end());
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
