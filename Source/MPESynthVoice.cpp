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

    if (allowTailOff) {
        if (tailOff == 0.0)
            tailOff = 1.0;
    }
    else {
        clearCurrentNote();
        phaseDelta = 0.0;
    }
}


void MPESynthVoice::notePressureChanged()
{
    float pressure = currentlyPlayingNote.pressure.asUnsignedFloat();
    level.setTargetValue(pressure);
    modulateDetune();
}


void MPESynthVoice::notePitchbendChanged()
{
    frequency.setTargetValue(currentlyPlayingNote.getFrequencyInHertz());
}


void MPESynthVoice::noteTimbreChanged()
{
    float timbreModVal = currentlyPlayingNote.timbre.asUnsignedFloat();
    timbre.setTargetValue(timbreModVal);
    
    float fQmax = 32.0;
    float onePoleMax = 0.001;
    
    float intensity = abs(timbreModVal - 0.5) * 2;
    float fQMod = (fQmax - baseFilterQ) * intensity;
    currentFilterQ = baseFilterQ + fQMod;
    float opMod = (onePoleMax - baseOnePoleFc) * intensity;
    currentOnePoleFc = baseOnePoleFc + opMod;
        
    if (timbreModVal > 0.5) {
        currentNoiseMix = baseNoiseMix - baseNoiseMix * intensity * 1.5;
        if (currentNoiseMix < 0) currentNoiseMix = 0;
    }
    else {
        currentNoiseMix = baseNoiseMix + (1 - baseNoiseMix) * intensity * 1.5;
        if (currentNoiseMix > 1) currentNoiseMix = 1;
    }
    
    modulateFilters();
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
        double f2 = f + 1.0005777895 * currentDetune;
        double tm = timbre.getNextValue();
        double lvl = level.getNextValue();
        
        tm = (tm - 0.5) * 1.25 + 0.5;
        if (tm > 1) tm = 1;
        if (tm < 0) tm = 0;
        
        double sawWaves = (osc1.saw(f) + osc2.saw(f2)) / 2;
        double mixedWave = sawWaves * (1 - currentNoiseMix) + oscNoise.noise() * currentNoiseMix;
        double theSound = mixedWave * lvl;
        
        if (tailOff > 0.0)
        {
            theSound *= tailOff;
            tailOff *= 0.99;
            if (tailOff <= 0.005)
            {
                clearCurrentNote();
                resetModulations();
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
        filteredCarrierBuffer[i].applyGain(currentFilterQ);
        
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


void MPESynthVoice::prepare(int numChannels, double sampleRate, int samplesPerBlock, float filterQ, float onePoleFc, float noiseMix)
{
    this->sampleRate = sampleRate;
    this->baseFilterQ = filterQ;
    this->baseOnePoleFc = onePoleFc;
    this->baseNoiseMix = noiseMix;
    
    inputBuffer.setSize(numChannels, samplesPerBlock);
    carrierBuffer.setSize(numChannels, samplesPerBlock);
    
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = numChannels;
    
    for (int i = 0; i < filterCount; ++i)
    {
        float frequency = filterFrequencies[round((filterFrequencies.size() / filterCount) * (i+1) - 1)];
        filteredCarrierBuffer[i].setSize(numChannels, samplesPerBlock);
        chosenFrequencies[i] = frequency;
        
        envFollowFilter[i] = new OnePole(onePoleFc);
        filteredModulatorBuffer[i].setSize(numChannels, samplesPerBlock);
    }
    
    resetModulations();
}


void MPESynthVoice::setFilterBaseQs(float filterQ, float onePoleFc)
{
    this->baseFilterQ = filterQ;
    this->baseOnePoleFc = onePoleFc;
    modulateFilters();
}


void MPESynthVoice::setMaxDetune(float maxDetune)
{
    this->maxDetune = maxDetune;
    modulateDetune();
}


void MPESynthVoice::setNoiseBaseMix(float noiseMix)
{
    this->baseNoiseMix = noiseMix;
}


void MPESynthVoice::modulateFilters()
{
    for (int i = 0; i < filterCount; ++i)
    {
        modulatorFilterBank[i].reset();
        *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], currentFilterQ);
        modulatorFilterBank[i].prepare(spec);
        
        carrierFilterBank[i].reset();
        *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], currentFilterQ);
        carrierFilterBank[i].prepare(spec);
        
        envFollowFilter[i]->setFc(currentOnePoleFc);
    }
}


void MPESynthVoice::resetModulations()
{
    currentNoiseMix = baseNoiseMix;
    currentFilterQ = baseFilterQ;
    currentOnePoleFc = baseOnePoleFc;
    modulateFilters();
}


void MPESynthVoice::modulateDetune()
{
    currentDetune = maxDetune * level.getNextValue();
}
