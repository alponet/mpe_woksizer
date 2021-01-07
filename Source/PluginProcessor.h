#pragma once

#include <JuceHeader.h>
#include "MPESynthVoice.h"
#include "OnePole.h"


/**
*/
class Mpe_woksizerAudioProcessor  : public AudioProcessor,
                                    private AudioProcessorValueTreeState::Listener
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
    
    const StringArray controllerParams;

private:
    double sampleRate;
    AudioProcessorValueTreeState parameters;
    MPESynthesiser synth;
    AudioBuffer<float> audioOutBuffer;
    static const int numVoices = 10;
        
    void parameterChanged (const String &parameterID, float newValue) override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mpe_woksizerAudioProcessor)
};
