#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
{
    addParameter(gain = new juce::AudioParameterFloat("gain", // parameterID
                                                      "Gain", // parameter name
                                                      0.0f,   // minimum value
                                                      1.0f,   // maximum value
                                                      0.5f)); // default value

    addParameter(pan_balance = new juce::AudioParameterFloat("pan",
                                                             "Pan",
                                                             0.0f,
                                                             1.0f,
                                                             0.5f));
    addParameter(invert_left = new juce::AudioParameterBool("invert_left", "L_Invert", false));
    addParameter(invert_right = new juce::AudioParameterBool("invert_right", "R_Invert", false));
    addParameter(mute = new juce::AudioParameterBool("mute", "Mute", false));
    addParameter(mono = new juce::AudioParameterBool("mono", "Mono", false));
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
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    *filter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 1000.0f);
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock), 2};
    filter.prepare(spec);
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages)
{
    auto n_i_channels = getNumInputChannels();
    auto n_o_channels = getNumOutputChannels();
    auto n_samples = buffer.getNumSamples();

    // clear output buffer if there is no corresponding input buffer
    for (int i = n_i_channels; i < n_o_channels; i++)
    {
        buffer.clear(i, 0, n_samples);
    }

    // gain & invert phase & mute
    buffer.applyGain(*gain * (*mute ? 0 : 1));

    // panning
    for (int i = 0; i < n_o_channels; i++)
    {
        auto left = (1.0f - *pan_balance) * (*invert_left ? -1.0f : 1.0f);
        auto right = (*pan_balance) * (*invert_right ? -1.0f : 1.0f);
        auto g = (i % 2 == 0) ? left : right;
        buffer.applyGain(i, 0, n_samples, g);
    }

    // filter
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
