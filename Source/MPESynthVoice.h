#pragma once

#include <JuceHeader.h>
#include "maximilian.h"
#include "OnePole.h"


class MPESynthVoice :  public MPESynthesiserVoice
{
public:
    MPESynthVoice(AudioProcessorValueTreeState& parameters);
    void noteStarted() override;
    void noteStopped (bool allowTailOff) override;
    void notePressureChanged() override;
    void notePitchbendChanged() override;
    void noteTimbreChanged() override;
    void noteKeyStateChanged() override {};
    void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    void setInputBuffer(AudioBuffer<float>& inputBuffer);
    void prepare(int numChannels, double sampleRate, int samplesPerBlock);
    void updateParamRanges();
private:
    AudioBuffer<float> inputBuffer;
    AudioBuffer<float> carrierBuffer;
    
    AudioProcessorValueTreeState* parameters = nullptr;
    
    SmoothedValue<double> level, timbre, frequency;
 
    double phase      = 0.0;
    double phaseDelta = 0.0;
    double tailOff    = 0.0;
    
    maxiOsc osc1;
    maxiOsc osc2;
    maxiOsc oscNoise;
    
    static const int filterCount = 22;
    
    array<float, 88> filterFrequencies = {
        27.5, 29.14, 30.87, 32.7, 34.65, 36.71, 38.89, 41.2, 43.65, 46.25, 49.0, 51.91, 55.0, 58.27, 61.74, 65.41, 69.3, 73.42, 77.78, 82.41, 87.31, 92.45, 98.0, 103.83, 110.0, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.0, 196.0, 207.65, 220.0, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.0, 415.3, 440.0, 466.16, 493.88, 523.25, 554.37, 587.33, 622.25, 659.26, 698.46, 739.99, 783.99, 830.61, 880.0, 932.33, 987.77, 1046.50, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91, 1479.98, 1567.98, 1661.22, 1760.0, 1864.66, 1975.53, 2093.0, 2217.46, 2349.32, 2489.32, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.0, 3729.31, 3951.07, 4186.01
    };
    array<float, filterCount> chosenFrequencies;
    
    array<dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>, filterCount> modulatorFilterBank;
    array<dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>>, filterCount> carrierFilterBank;
    
    array<AudioBuffer<float>, filterCount> filteredCarrierBuffer;
    array<AudioBuffer<float>, filterCount> filteredModulatorBuffer;
    array<OnePole*, filterCount> envFollowFilter;
    
    double sampleRate;
    dsp::ProcessSpec spec;
    
    NormalisableRange<float> filterQRange;
    NormalisableRange<float> envFollowerRange;
    NormalisableRange<float> detuneRange;
    NormalisableRange<float> noiseRange;
    
    bool isFilterQRangeInverted = false;
    bool isEnvFollowerRangeInverted = false;
    bool isDetuneRangeInverted = false;
    bool isNoiseRangeInverted = false;
    
    SmoothedValue<double> currentFilterQ;
    SmoothedValue<double> currentEnvFollowerFc;
    SmoothedValue<double> currentDetune;
    SmoothedValue<double> currentNoiseMix;
    
    void modulateParamByController(int controlID, float intensity);
    
    void modulateFilterQ(float intensity);
    void modulateEnvFollower(float intensity);
    void modulateDetune(float intensity);
    void modulateNoise(float intensity);
};
