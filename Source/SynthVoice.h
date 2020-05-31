#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "maximilian.h"


class SynthVoice : public SynthesiserVoice
{
public:
    bool canPlaySound (SynthesiserSound* sound);
    void startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition);
    void stopNote (float velocity, bool allowTailOff);
    void pitchWheelMoved (int newPitchWheelValue) {};
    void controllerMoved (int controllerNumber, int newControllerValue) {};
    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples);
    void setEnvelope (float a, float d, float s, float r);
    void setDetune (float cents);
private:
    double level;
    double frequency;
    double detune;
    
    maxiOsc osc;
    maxiEnv env;
};
