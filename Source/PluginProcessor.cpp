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
        std::make_unique<AudioParameterFloat> ("filterQ", "FilterQ", NormalisableRange<float> (0.1, 32.0, 0.01, 0.5), 14.0),
        std::make_unique<AudioParameterFloat> ("onePoleFc", "OnePoleFc", NormalisableRange<float> (0.000001, 0.001, 0.0000001, 0.5), 0.00005),
        std::make_unique<AudioParameterFloat> ("osc2Detune", "Osc2Detune", NormalisableRange<float> (-100.0, 100.0, 0.1, 1.0), 10.0),
        std::make_unique<AudioParameterFloat> ("oscNoiseLevel", "OscNoiseLevel", NormalisableRange<float> (0.0, 1.2, 0.01, 0.5), 0.1),
    })
{
    for (int i = 0; i < this->numVoices; i++) {
        synth.addVoice(new MPESynthVoice());
    }
    
    parameters.addParameterListener("filterQ", this);
    parameters.addParameterListener("onePoleFc", this);
    parameters.addParameterListener("osc2Detune", this);
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
        voice->setupFilters(getTotalNumOutputChannels(), sampleRate, samplesPerBlock);
    }
    
    audioOutBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    
    /*
    float filterQ = *parameters.getRawParameterValue("filterQ");
    float onePoleFc = *parameters.getRawParameterValue("onePoleFc");
    for (int i = 0; i < filterCount; ++i)
    {
        float frequency = filterFrequencies[round((filterFrequencies.size() / filterCount) * (i+1) - 1)];
        filteredCarrierBuffer[i].setSize(getTotalNumInputChannels(), samplesPerBlock);
        
        modulatorFilterBank[i].reset();
        *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, frequency, filterQ);
        modulatorFilterBank[i].prepare(spec);
        
        carrierFilterBank[i].reset();
        *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, frequency, filterQ);
        carrierFilterBank[i].prepare(spec);
        
        chosenFrequencies[i] = frequency;
        
        envFollowFilter[i] = new OnePole(onePoleFc);
        filteredModulatorBuffer[i].setSize(getTotalNumInputChannels(), samplesPerBlock);
    }
    */
    
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
    if (parameterID == "filterQ")
    {
//        for (int i = 0; i < modulatorFilterBank.size(); ++i)
//        {
//            *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], *parameters.getRawParameterValue("filterQ"));
//            *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], *parameters.getRawParameterValue("filterQ"));
//        }
        
    }
    else if (parameterID == "onePoleFc")
    {
//        for (int i = 0; i < envFollowFilter.size(); ++i)
//        {
//            envFollowFilter[i]->setFc(*parameters.getRawParameterValue("onePoleFc"));
//        }
        
    }
    else if (parameterID == "osc2Detune")
    {
//        for (int i = 0; i < osc2.getNumVoices(); ++i) {
//            static_cast<SynthVoice*>(osc2.getVoice(i))->setDetune(*parameters.getRawParameterValue("osc2Detune"));
//        }
        
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
