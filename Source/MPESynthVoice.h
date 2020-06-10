#pragma once

#include <JuceHeader.h>


class MPESynthVoice :  public MPESynthesiserVoice
{
public:
    void noteStarted() override;
    void noteStopped (bool allowTailOff) override;
    void notePressureChanged() override;
    void notePitchbendChanged() override;
    void noteTimbreChanged() override;
    void noteKeyStateChanged() override {};
    void renderNextBlock(AudioBuffer<float>& audioBuffer, int startSample, int numSamples) override;
private:
    SmoothedValue<double> level, timbre, frequency;
 
    double phase      = 0.0;
    double phaseDelta = 0.0;
    double tailOff    = 0.0;
 
    // some useful constants
    static constexpr auto maxLevel = 0.05;
    static constexpr auto maxLevelDb = 31.0;
    static constexpr auto smoothingLengthInSeconds = 0.01;
    
    
};
