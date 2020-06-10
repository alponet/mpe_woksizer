#include "MPESynthVoice.h"


void MPESynthVoice::noteStarted()
{
    jassert (currentlyPlayingNote.isValid());
    jassert (currentlyPlayingNote.keyState == MPENote::keyDown || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);
 
    // get data from the current MPENote
    level.setTargetValue (currentlyPlayingNote.pressure.asUnsignedFloat());
    frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
    timbre.setTargetValue (currentlyPlayingNote.timbre.asUnsignedFloat());
 
    phase = 0.0;
    auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
    phaseDelta = 2.0 * MathConstants<double>::pi * cyclesPerSample;
 
    tailOff = 0.0;
}


void MPESynthVoice::noteStopped (bool allowTailOff)
{
    jassert (currentlyPlayingNote.keyState == MPENote::off);

    if (allowTailOff)
    {
        // start a tail-off by setting this flag. The render callback will pick up on
        // this and do a fade out, calling clearCurrentNote() when it's finished.

        if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
            // stopNote method could be called more than once.
            tailOff = 1.0;
    }
    else
    {
        // we're being told to stop playing immediately, so reset everything..
        clearCurrentNote();
        phaseDelta = 0.0;
    }
}


void MPESynthVoice::notePressureChanged()
{
    // ToDo
    
    auto pr = currentlyPlayingNote.pressure.asUnsignedFloat();
}


void MPESynthVoice::notePitchbendChanged()
{
    // ToDo
    
    auto f = currentlyPlayingNote.getFrequencyInHertz();
}


void MPESynthVoice::noteTimbreChanged()
{
    // ToDo
    
    auto tm = currentlyPlayingNote.timbre.asUnsignedFloat();
}


void MPESynthVoice::renderNextBlock(AudioBuffer<float>& audioBuffer, int startSample, int numSamples)
{
    // ToDo
}
