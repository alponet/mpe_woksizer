#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "SynthVoice.h"


/**
*/
class Mpe_woksizerAudioProcessor  : public AudioProcessor
{
public:    
    Mpe_woksizerAudioProcessor();
    ~Mpe_woksizerAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    AudioProcessorValueTreeState parameters;
    
    Synthesiser mySynth;
    SynthVoice* myVoice;
    
    AudioBuffer<float> audioAnalyseBuffer;
    AudioBuffer<float> audioOutBuffer;
        
    static const int filterCount = 22;
    static constexpr float filterQ = 3.5f;
    
    array<float, 88> filterFrequencies = { 27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49.0, 51.91, 55.0, 58.27, 61.74, 65.41, 69.3, 73.42, 77.78, 82.41, 87.31, 92.45, 98.0, 103.83, 110.0, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.0, 196.0, 207.65, 220.0, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.0, 415.3, 440.0, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61, 880.0, 932.33, 987.77, 1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760.0, 1864.66, 1975.53, 2093.0, 2217.46, 2349.32, 2489.32, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.0, 3729.31, 3951.07, 4186.01
    };
    
    array<dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>, filterCount> modulatorFilterBank;
    array<dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>, filterCount> carrierFilterBank;
    
    array<float, filterCount> envFollower;
    array<AudioBuffer<float>, filterCount> filteredCarrierBuffer;
    array<float, filterCount> lastGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mpe_woksizerAudioProcessor)
};
