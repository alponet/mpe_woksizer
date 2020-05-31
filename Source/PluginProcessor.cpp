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
        std::make_unique<AudioParameterFloat> ("envAttack", "Attack", NormalisableRange<float> (0.0, 6000.0, 1.0, 0.5), 80.0),
        std::make_unique<AudioParameterFloat> ("envRelease", "Release", NormalisableRange<float> (0.0, 6000.0, 1.0, 0.5), 200.0),
        std::make_unique<AudioParameterFloat> ("filterQ", "FilterQ", NormalisableRange<float> (0.1, 32.0, 0.01, 0.5), 14.0),
        std::make_unique<AudioParameterFloat> ("onePoleFc", "OnePoleFc", NormalisableRange<float> (0.000001, 0.001, 0.0000001, 0.5), 0.00005),
        std::make_unique<AudioParameterFloat> ("osc1Level", "Osc1Level", NormalisableRange<float> (0.0, 1.2, 0.01, 0.5), 0.8),
        std::make_unique<AudioParameterFloat> ("osc2Detune", "Osc2Detune", NormalisableRange<float> (-100.0, 100.0, 0.1, 1.0), 10.0),
        std::make_unique<AudioParameterFloat> ("osc2Level", "Osc2Level", NormalisableRange<float> (0.0, 1.2, 0.01, 0.5), 0.3),
        std::make_unique<AudioParameterFloat> ("oscNoiseLevel", "OscNoiseLevel", NormalisableRange<float> (0.0, 1.2, 0.01, 0.5), 0.1),
    })
{
    osc1.clearVoices();
    osc2.clearVoices();
    oscNoise.clearVoices();
    for (int i = 0; i < this->numVoices; i++) {
        osc1.addVoice (new SynthVoice());
        osc2.addVoice (new SynthVoice());
        oscNoise.addVoice(new NoiseVoice());
    }
    
    osc1.clearSounds();
    osc2.clearSounds();
    oscNoise.clearSounds();
    osc1.addSound(new SynthSound());
    osc2.addSound(new SynthSound());
    oscNoise.addSound(new SynthSound());
    
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
    
    ignoreUnused(samplesPerBlock);
    osc1.setCurrentPlaybackSampleRate(sampleRate);
    osc2.setCurrentPlaybackSampleRate(sampleRate);
    oscNoise.setCurrentPlaybackSampleRate(sampleRate);
    audioAnalyseBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    audioOutBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    osc1Buffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    osc2Buffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    noiseBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
    
    for (int i = 0; i < osc2.getNumVoices(); ++i) {
        static_cast<SynthVoice*> (osc2.getVoice(i))->setDetune(*parameters.getRawParameterValue("osc2Detune"));
    }

    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
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

    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    float a = *parameters.getRawParameterValue("envAttack");
    float d = 100.0;
    float s = 0.8;
    float r = *parameters.getRawParameterValue("envRelease");
    for (int i = 0; i < this->numVoices; ++i) {
        if ((osc1Voice = dynamic_cast<SynthVoice*>(osc1.getVoice(i))))
            osc1Voice->setEnvelope(a, d, s, r);
        if ((osc2Voice = dynamic_cast<SynthVoice*>(osc2.getVoice(i))))
            osc2Voice->setEnvelope(a, d, s, r);
        if ((noiseVoice = dynamic_cast<NoiseVoice*>(oscNoise.getVoice(i))))
            noiseVoice->setEnvelope(a, d, s, r);
    }

    dsp::AudioBlock<float> audioInputBlock(buffer);
    for (int i = 0; i < filterCount; ++i)
    {
        filteredModulatorBuffer[i].clear();
        dsp::AudioBlock<float> modulatorBlock(filteredModulatorBuffer[i]);
        modulatorFilterBank[i].process(dsp::ProcessContextNonReplacing<float>(audioInputBlock, modulatorBlock));
    }

    audioOutBuffer.clear();
    buffer.clear();
    osc1Buffer.clear();
    osc2Buffer.clear();
    noiseBuffer.clear();
    
    osc1.renderNextBlock(osc1Buffer, midiMessages, 0, osc1Buffer.getNumSamples());    
    osc2.renderNextBlock(osc2Buffer, midiMessages, 0, osc2Buffer.getNumSamples());
    oscNoise.renderNextBlock(noiseBuffer, midiMessages, 0, noiseBuffer.getNumSamples());
    osc1Buffer.applyGain(*parameters.getRawParameterValue("osc1Level"));
    osc2Buffer.applyGain(*parameters.getRawParameterValue("osc2Level"));
    noiseBuffer.applyGain(*parameters.getRawParameterValue("oscNoiseLevel"));
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        buffer.addFrom(channel, 0, osc1Buffer, channel, 0, osc1Buffer.getNumSamples());
        buffer.addFrom(channel, 0, osc2Buffer, channel, 0, osc2Buffer.getNumSamples());
        buffer.addFrom(channel, 0, noiseBuffer, channel, 0, noiseBuffer.getNumSamples());
    }
    
    for (int i = 0; i < filterCount; ++i)
    {
        filteredCarrierBuffer[i].clear();
        dsp::AudioBlock<float> filterBlock(filteredCarrierBuffer[i]);
        filterBlock.copyFrom(audioInputBlock);
        carrierFilterBank[i].process(dsp::ProcessContextReplacing<float> (filterBlock));
        filteredCarrierBuffer[i].applyGain(*parameters.getRawParameterValue("filterQ"));
        
        for (int channel = 0; channel < filteredModulatorBuffer[i].getNumChannels(); ++channel)
        {
            auto* modulatorBuffer = filteredModulatorBuffer[i].getReadPointer(channel);
            auto* carrierBuffer = filteredCarrierBuffer[i].getWritePointer(channel);
            
            for (auto sample = 0; sample < filteredCarrierBuffer[i].getNumSamples(); ++sample)
            {
                carrierBuffer[sample] *= envFollowFilter[i]->process(abs(modulatorBuffer[sample]));
            }
            
            audioOutBuffer.addFrom(channel, 0, filteredCarrierBuffer[i], channel, 0, filteredCarrierBuffer[i].getNumSamples());
        }
    }
    
    buffer = audioOutBuffer;
    buffer.applyGain(*parameters.getRawParameterValue("volume"));
}


void Mpe_woksizerAudioProcessor::parameterChanged (const String &parameterID, float newValue)
{    
    if (parameterID == "filterQ")
    {
        for (int i = 0; i < modulatorFilterBank.size(); ++i)
        {
            *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], *parameters.getRawParameterValue("filterQ"));
            *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], *parameters.getRawParameterValue("filterQ"));
        }
    }
    else if (parameterID == "onePoleFc")
    {
        for (int i = 0; i < envFollowFilter.size(); ++i)
        {
            envFollowFilter[i]->setFc(*parameters.getRawParameterValue("onePoleFc"));
        }
    }
    else if (parameterID == "osc2Detune")
    {
        for (int i = 0; i < osc2.getNumVoices(); ++i) {
            static_cast<SynthVoice*>(osc2.getVoice(i))->setDetune(*parameters.getRawParameterValue("osc2Detune"));
        }
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
