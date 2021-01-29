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
    controllerParams({
       "", "Pressure", "Timbre"
    }),
    parameters (*this, nullptr, Identifier ("WoksizerParams"),
    {
        std::make_unique<AudioParameterFloat> ("volume", "Volume", NormalisableRange<float> (0.0, 2.0, 0.001, 0.5), 1.0),
        std::make_unique<AudioParameterFloat> ("filterQMin", "Filter Q min", NormalisableRange<float> (0.1, 8.0, 0.01, 0.5), 4.0),
        std::make_unique<AudioParameterFloat> ("filterQMax", "Filter Q max", NormalisableRange<float> (0.1, 8.0, 0.01, 0.5), 6.0),
        std::make_unique<AudioParameterChoice> ("filterQControl", "Filter Q Controller", controllerParams, 0),
        std::make_unique<AudioParameterFloat> ("envFollowerMin", "Envelope Follower min", NormalisableRange<float> (0.000001, 0.001, 0.0000001, 0.5), 0.00005),
        std::make_unique<AudioParameterFloat> ("envFollowerMax", "Envelope Follower max", NormalisableRange<float> (0.000001, 0.001, 0.0000001, 0.5), 0.0001),
        std::make_unique<AudioParameterChoice> ("envFollowerControl", "Envelope Follower Controller", controllerParams, 0),
        std::make_unique<AudioParameterFloat> ("detuneMin", "osc2 Detune min", NormalisableRange<float> (-100.0, 100.0, 0.1, 1.0), 0.0),
        std::make_unique<AudioParameterFloat> ("detuneMax", "osc2 Detune max", NormalisableRange<float> (-100.0, 100.0, 0.1, 1.0), 20.0),
        std::make_unique<AudioParameterChoice> ("detuneControl", "Detune Controller", controllerParams, 0),
        std::make_unique<AudioParameterFloat> ("noiseMin", "noise min", NormalisableRange<float> (0.0, 1.2, 0.01, 1.0), 0.0),
        std::make_unique<AudioParameterFloat> ("noiseMax", "noise max", NormalisableRange<float> (0.0, 1.2, 0.01, 1.0), 0.2),
        std::make_unique<AudioParameterChoice> ("noiseControl", "Noise Controller", controllerParams, 0),
    })
{
    for (int i = 0; i < this->numVoices; i++) {
        synth.addVoice(new MPESynthVoice(parameters));
    }
    
    parameters.addParameterListener("filterQMin", this);
    parameters.addParameterListener("filterQMax", this);
    parameters.addParameterListener("filterQControl", this);
    parameters.addParameterListener("envFollowerMin", this);
    parameters.addParameterListener("envFollowerMax", this);
    parameters.addParameterListener("envFollowerControl", this);
    parameters.addParameterListener("detuneMin", this);
    parameters.addParameterListener("detuneMax", this);
    parameters.addParameterListener("detuneControl", this);
    parameters.addParameterListener("noiseMin", this);
    parameters.addParameterListener("noiseMax", this);
    parameters.addParameterListener("noiseControl", this);
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
    this->sampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);
    
    for (int i = 0; i < synth.getNumVoices(); ++i) {
        MPESynthVoice* voice = static_cast<MPESynthVoice*>(synth.getVoice(i));
        
        voice->prepare(getTotalNumOutputChannels(), sampleRate, samplesPerBlock);
    }
    
    audioOutBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
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
    
    for (int i = 0; i < synth.getNumVoices(); ++i) {
        MPESynthVoice* voice = static_cast<MPESynthVoice*>(synth.getVoice(i));
        voice->setInputBuffer(buffer);
    }
    
    audioOutBuffer.clear();
    synth.renderNextBlock(audioOutBuffer, midiMessages, 0, buffer.getNumSamples());
    midiMessages.clear();
    
    buffer = audioOutBuffer;
    buffer.applyGain(*parameters.getRawParameterValue("volume"));
}


void Mpe_woksizerAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        static_cast<MPESynthVoice*>(synth.getVoice(i))->updateParamRanges();
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
