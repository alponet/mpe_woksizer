#include "MPESynthVoice.h"


void MPESynthVoice::noteStarted()
{
    jassert (currentlyPlayingNote.isValid());
    jassert (currentlyPlayingNote.keyState == MPENote::keyDown || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);
 
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
        if (tailOff == 0.0)
            tailOff = 1.0;
    }
    else
    {
        clearCurrentNote();
        phaseDelta = 0.0;
    }
}


void MPESynthVoice::notePressureChanged()
{
    level.setTargetValue(currentlyPlayingNote.pressure.asUnsignedFloat());
}


void MPESynthVoice::notePitchbendChanged()
{
    frequency.setTargetValue(currentlyPlayingNote.getFrequencyInHertz());
}


void MPESynthVoice::noteTimbreChanged()
{
    timbre.setTargetValue(currentlyPlayingNote.timbre.asUnsignedFloat());
    
    // ToDo: modulate FilterQ and onePole
}


void MPESynthVoice::renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    dsp::AudioBlock<float> audioInputBlock(inputBuffer);
    for (int i = 0; i < filterCount; ++i)
    {
        filteredModulatorBuffer[i].clear();
        dsp::AudioBlock<float> modulatorBlock(filteredModulatorBuffer[i]);
        modulatorFilterBank[i].process(dsp::ProcessContextNonReplacing<float>(audioInputBlock, modulatorBlock));
    }
    
    carrierBuffer.clear();
    for (int sample = 0; sample < numSamples; ++sample)
    {
        double f = frequency.getNextValue();
        double tm = timbre.getNextValue();
        double lvl = level.getNextValue();
        
        tm = (tm - 0.5) * 1.25 + 0.5;
        if (tm > 1) tm = 1;
        if (tm < 0) tm = 0;
        
        double f2 = f + lvl * 1.0005777895 * 5;
        
        double sawWaves = (osc1.saw(f) + osc2.saw(f2)) / 2;
        double noiseLvl = (0.5 - tm) > 0 ? (0.5 - tm) : 0;
        double mixedWave = sawWaves * tm + oscNoise.noise() * noiseLvl;
        double theSound = mixedWave * lvl;
        
        if (tailOff > 0.0)
        {
            theSound *= tailOff;
            tailOff *= 0.99;
            if (tailOff <= 0.005)
            {
                clearCurrentNote();
                phaseDelta = 0.0;
            }
        }
        
        for (int channel = 0; channel < carrierBuffer.getNumChannels(); ++channel)
        {
            carrierBuffer.addSample(channel, startSample, theSound);
        }
        
        ++startSample;
    }
    
    dsp::AudioBlock<float> cleanCarrierBlock(carrierBuffer);
    for (int i = 0; i < filterCount; ++i)
    {
        filteredCarrierBuffer[i].clear();
        dsp::AudioBlock<float> filteredCarrierBlock(filteredCarrierBuffer[i]);
        filteredCarrierBlock.copyFrom(cleanCarrierBlock);
        carrierFilterBank[i].process(dsp::ProcessContextReplacing<float> (filteredCarrierBlock));
        filteredCarrierBuffer[i].applyGain(filterQ);
        
        for (int channel = 0; channel < filteredModulatorBuffer[i].getNumChannels(); ++channel)
        {
            auto* modulator = filteredModulatorBuffer[i].getReadPointer(channel);
            auto* carrier = filteredCarrierBuffer[i].getWritePointer(channel);
            
            for (auto sample = 0; sample < filteredCarrierBuffer[i].getNumSamples(); ++sample)
            {
                carrier[sample] *= envFollowFilter[i]->process(abs(modulator[sample]));
            }
            
            outputBuffer.addFrom(channel, 0, filteredCarrierBuffer[i], channel, 0, filteredCarrierBuffer[i].getNumSamples());
        }
    }
}


void MPESynthVoice::setInputBuffer(AudioBuffer<float> &inputBuffer)
{
    this->inputBuffer = inputBuffer;
}


void MPESynthVoice::setupFilters(int numChannels, double sampleRate, int samplesPerBlock)
{
    inputBuffer.setSize(numChannels, samplesPerBlock);
    carrierBuffer.setSize(numChannels, samplesPerBlock);
    
    dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = numChannels;
    
    filterQ = 14.0;
    onePoleFc = 0.00005;
    
    // ToDo: get values from value tree state
    
    for (int i = 0; i < filterCount; ++i)
    {
        float frequency = filterFrequencies[round((filterFrequencies.size() / filterCount) * (i+1) - 1)];
        filteredCarrierBuffer[i].setSize(numChannels, samplesPerBlock);
        
        modulatorFilterBank[i].reset();
        *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, frequency, filterQ);
        modulatorFilterBank[i].prepare(spec);
        
        carrierFilterBank[i].reset();
        *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, frequency, filterQ);
        carrierFilterBank[i].prepare(spec);
        
        chosenFrequencies[i] = frequency;
        
        envFollowFilter[i] = new OnePole(onePoleFc);
        filteredModulatorBuffer[i].setSize(numChannels, samplesPerBlock);
    }
}
