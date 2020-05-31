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
    for (int sample = 0; sample < numSamples; ++sample)
    {
        double detunedFrequency = frequency + this->detune * 1.0005777895;
        double theWave = osc.saw(detunedFrequency);
        double theSound = env.adsr(theWave, env.trigger) * level;
        
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            outputBuffer.addSample(channel, startSample, theSound);
        }
        
        ++startSample;
    }
}


void SynthVoice::setEnvelope(float a, float d, float s, float r)
{
    env.setAttack(a);
    env.setDecay(d);
    env.setSustain(s);
    env.setRelease(r);
}


void SynthVoice::setDetune(float cents)
{
    detune = cents;
}
