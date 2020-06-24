#include "MPESynthVoice.h"


void MPESynthVoice::noteStarted()
{
    jassert (currentlyPlayingNote.isValid());
    jassert (currentlyPlayingNote.keyState == MPENote::keyDown || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);
 
    level.setTargetValue (currentlyPlayingNote.pressure.asUnsignedFloat());
    frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
    timbre.setTargetValue (currentlyPlayingNote.timbre.asUnsignedFloat());
 
    phase = 0.0;
    auto cyclesPerSample = frequency.getTargetValue() / currentSampleRate;
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
    modulateNoiseMix();
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
        double f2 = f + 1.0005777895 * currentDetune.getNextValue();
        double tm = timbre.getNextValue();
        double lvl = level.getNextValue();
        
        tm = (tm - 0.5) * 1.25 + 0.5;
        if (tm > 1) tm = 1;
        if (tm < 0) tm = 0;
        
        double sawWaves = (osc1.saw(f) + osc2.saw(f2)) / 2;
        double noiseMix = currentNoiseMix.getNextValue();
        double mixedWave = sawWaves * (1 - noiseMix) + oscNoise.noise() * noiseMix;
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

    dsp::AudioBlock<float> dryCarrierBlock(carrierBuffer);
    double filterQ = currentFilterQ.getNextValue();
    for (int i = 0; i < filterCount; ++i)
    {
        filteredCarrierBuffer[i].clear();
        dsp::AudioBlock<float> filteredCarrierBlock(filteredCarrierBuffer[i]);
        filteredCarrierBlock.copyFrom(dryCarrierBlock);
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


void MPESynthVoice::prepare(int numChannels, double sampleRate, int samplesPerBlock, float filterQ, float onePoleFc, float noiseMix)
{
    this->sampleRate = sampleRate;
    this->baseFilterQ = filterQ;
    this->baseOnePoleFc = onePoleFc;
    this->baseNoiseMix = noiseMix;
    
    inputBuffer.setSize(numChannels, samplesPerBlock);
    carrierBuffer.setSize(numChannels, samplesPerBlock);
    
    level.reset(100);
    currentNoiseMix.reset(100);
    
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
        
        modulatorFilterBank[i].reset();
        *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], filterQ);
        modulatorFilterBank[i].prepare(spec);
                
        carrierFilterBank[i].reset();
        *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], filterQ);
        carrierFilterBank[i].prepare(spec);
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
    modulateNoiseMix();
}


void MPESynthVoice::modulateFilters()
{
    double fQmax = 18.0;
    double onePoleMax = 0.001;
    
    double timbreMod = timbre.getTargetValue();
    double intensity = abs(timbreMod - 0.5) * 2;
    double fQMod = (fQmax - baseFilterQ) * intensity;
    currentFilterQ.setTargetValue(baseFilterQ + fQMod);
    double opMod = (onePoleMax - baseOnePoleFc) * intensity;
    currentOnePoleFc.setTargetValue(baseOnePoleFc + opMod);
    
    double filterQ = currentFilterQ.getTargetValue();
    double onePoleFc = currentOnePoleFc.getTargetValue();
    for (int i = 0; i < filterCount; ++i)
    {
        *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], filterQ);
        *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], filterQ);
        envFollowFilter[i]->setFc(onePoleFc);
    }
}


void MPESynthVoice::modulateNoiseMix()
{
    double noiseMix;
    double timbreMod = timbre.getTargetValue();
    double intensity = abs(timbreMod - 0.5) * 2;
    if (timbreMod > 0.5) {
        noiseMix = baseNoiseMix - baseNoiseMix * intensity * 1.5;
        if (noiseMix < 0) noiseMix = 0;
    }
    else {
        noiseMix = baseNoiseMix + (1 - baseNoiseMix) * intensity * 1.5;
        if (noiseMix > 1) noiseMix = 1;
    }
    currentNoiseMix.setTargetValue(noiseMix);
}


void MPESynthVoice::resetModulations()
{
    currentNoiseMix.setCurrentAndTargetValue(baseNoiseMix);
    currentFilterQ.setCurrentAndTargetValue(baseFilterQ);
    currentOnePoleFc.setCurrentAndTargetValue(baseOnePoleFc);
    modulateFilters();
}


void MPESynthVoice::modulateDetune()
{
    currentDetune.setTargetValue(maxDetune * level.getTargetValue());
}
