#include "PluginProcessor.h"
#include "PluginEditor.h"


Mpe_woksizerAudioProcessor::Mpe_woksizerAudioProcessor()  :
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor (BusesProperties()
         #if ! JucePlugin_IsMidiEffect
          #if ! JucePlugin_IsSynth
           .withInput  ("Input",  AudioChannelSet::stereo(), true)
          #endif
           .withOutput ("Output", AudioChannelSet::stereo(), true)
         #endif
           ),
#endif
    parameters (*this, nullptr, Identifier ("WoksizerParams"),
    {
        std::make_unique<AudioParameterFloat> ("volume", "Volume", NormalisableRange<float> (0.0, 2.0, 0.01, 0.5), 1.0),
        std::make_unique<AudioParameterFloat> ("envAttack", "Attack", NormalisableRange<float> (0.0, 6000.0, 1.0, 0.5), 100.0),
        std::make_unique<AudioParameterFloat> ("envDecay", "Decay", NormalisableRange<float> (0.0, 6000.0, 1.0, 0.5), 100.0),
        std::make_unique<AudioParameterFloat> ("envSustain", "Sustain", NormalisableRange<float> (0.0, 1.0, 0.01, 0.5), 0.8),
        std::make_unique<AudioParameterFloat> ("envRelease", "Release", NormalisableRange<float> (0.0, 6000.0, 1.0, 0.5), 500.0)
    })
{
    mySynth.clearVoices();
    for (int i = 0; i < 5; i++) {
        mySynth.addVoice (new SynthVoice());
    }
    
    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
}


Mpe_woksizerAudioProcessor::~Mpe_woksizerAudioProcessor()
{
}


const String Mpe_woksizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}


bool Mpe_woksizerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}


bool Mpe_woksizerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}


bool Mpe_woksizerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}


double Mpe_woksizerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}


int Mpe_woksizerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}


int Mpe_woksizerAudioProcessor::getCurrentProgram()
{
    return 0;
}


void Mpe_woksizerAudioProcessor::setCurrentProgram (int index)
{
}


const String Mpe_woksizerAudioProcessor::getProgramName (int index)
{
    return {};
}


void Mpe_woksizerAudioProcessor::changeProgramName (int index, const String& newName)
{
}


void Mpe_woksizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    ignoreUnused(samplesPerBlock);
    lastSampleRate = sampleRate;
    mySynth.setCurrentPlaybackSampleRate(lastSampleRate);
}


void Mpe_woksizerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}


#ifndef JucePlugin_PreferredChannelConfigurations
bool Mpe_woksizerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void Mpe_woksizerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (int i = 0; i < mySynth.getNumVoices(); i++)
    {
        if ((myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(i)))) {
            float a = *parameters.getRawParameterValue("envAttack");
            float d = *parameters.getRawParameterValue("envDecay");
            float s = *parameters.getRawParameterValue("envSustain");
            float r = *parameters.getRawParameterValue("envRelease");
            myVoice->setEnvelope(a, d, s, r);
        }
    }
    
    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    
    for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
            channelData[sample] *= *parameters.getRawParameterValue("volume");
    }
    
}


bool Mpe_woksizerAudioProcessor::hasEditor() const
{
    return true;
}


AudioProcessorEditor* Mpe_woksizerAudioProcessor::createEditor()
{
    return new Mpe_woksizerAudioProcessorEditor (*this, parameters);
}


void Mpe_woksizerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary(*xml, destData);
}


void Mpe_woksizerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}


// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Mpe_woksizerAudioProcessor();
}
