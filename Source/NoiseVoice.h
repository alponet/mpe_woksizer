#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "maximilian.h"


class NoiseVoice : public SynthesiserVoice
{
public:
    bool canPlaySound (SynthesiserSound* sound);
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition);
    void stopNote (float velocity, bool allowTailOff);
    void pitchWheelMoved (int newPitchWheelValue) {};
    void controllerMoved (int controllerNumber, int newControllerValue) {};
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples);
    void setEnvelope (float a, float d, float s, float r);
private:
    double level;    
    double theWave;
    
    maxiOsc osc;
    maxiEnv env;
};
