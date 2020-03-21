#include "SynthVoice.h"


bool SynthVoice::canPlaySound (SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}


void SynthVoice::startNote (int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
    level = velocity;
    frequency = MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    env.trigger = 1;
}


void SynthVoice::stopNote (float velocity, bool allowTailOff)
{
    env.trigger = 0;
    clearCurrentNote();
}


void SynthVoice::renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    env.setAttack(100);
    env.setDecay(100);
    env.setSustain(0.8);
    env.setRelease(500);
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        double theWave = osc.sinewave(frequency);
        double theSound = env.adsr(theWave, env.trigger) * level;
        
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addSample(channel, startSample, theSound);
        }
        
        ++startSample;
    }
}
