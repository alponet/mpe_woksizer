#include "NoiseVoice.h"


bool NoiseVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}


void NoiseVoice::startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
    level = velocity;
    env.trigger = 1;
}


void NoiseVoice::stopNote (float velocity, bool allowTailOff)
{
    env.trigger = 0;
    clearCurrentNote();
}


void NoiseVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    for (int sample = 0; sample < numSamples; ++sample)
    {
        theWave = osc.noise();
        double theSound = env.adsr(theWave, env.trigger) * level;
        
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addSample(channel, startSample, theSound);
        }
        
        ++startSample;
    }
}


void NoiseVoice::setEnvelope(float a, float d, float s, float r)
{
    env.setAttack(a);
    env.setDecay(d);
    env.setSustain(s);
    env.setRelease(r);
}
