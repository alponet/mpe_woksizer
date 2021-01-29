#include "MPESynthVoice.h"


MPESynthVoice::MPESynthVoice(AudioProcessorValueTreeState& parameters) {
    this->parameters = &parameters;
}


void MPESynthVoice::noteStarted()
{
    jassert (currentlyPlayingNote.isValid());
    jassert (currentlyPlayingNote.keyState == MPENote::keyDown || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);

    level.setTargetValue (currentlyPlayingNote.noteOnVelocity.asUnsignedFloat());
    frequency.setTargetValue (currentlyPlayingNote.getFrequencyInHertz());
 
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
    modulateParamByController(1, pressure);
    level.setTargetValue(pressure);
}


void MPESynthVoice::notePitchbendChanged()
{
    frequency.setTargetValue(currentlyPlayingNote.getFrequencyInHertz());
}


void MPESynthVoice::noteTimbreChanged()
{
    float timbreModVal = currentlyPlayingNote.timbre.asUnsignedFloat();
    modulateParamByController(2, timbreModVal);
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
        double lvl = level.getNextValue();
        
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
        
        // using only channel 0 here, because the envFollowFilter does not have multi-channel support yet
        auto* modulator = filteredModulatorBuffer[i].getReadPointer(0);
        auto* carrier = filteredCarrierBuffer[i].getWritePointer(0);
        
        for (auto sample = 0; sample < filteredCarrierBuffer[i].getNumSamples(); ++sample)
        {
            carrier[sample] *= envFollowFilter[i]->process(abs(modulator[sample]));
        }
        
        for (int channel = 0; channel < filteredModulatorBuffer[i].getNumChannels(); ++channel)
        {
            outputBuffer.addFrom(channel, 0, filteredCarrierBuffer[i], 0, 0, filteredCarrierBuffer[i].getNumSamples());
        }
    }
}


void MPESynthVoice::setInputBuffer(AudioBuffer<float> &inputBuffer)
{
    this->inputBuffer = inputBuffer;
}


void MPESynthVoice::prepare(int numChannels, double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    
    inputBuffer.setSize(numChannels, samplesPerBlock);
    carrierBuffer.setSize(numChannels, samplesPerBlock);
    
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = numChannels;
    
    for (int i = 0; i < filterCount; ++i)
    {
        float frequency = filterFrequencies[round((filterFrequencies.size() / filterCount) * (i+1) - 1)];
        filteredCarrierBuffer[i].setSize(numChannels, samplesPerBlock);
        chosenFrequencies[i] = frequency;
        
        envFollowFilter[i] = new OnePole(*(parameters->getRawParameterValue("envFollowerMin")));
        filteredModulatorBuffer[i].setSize(numChannels, samplesPerBlock);
        
        modulatorFilterBank[i].reset();
        *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], *(parameters->getRawParameterValue("filterQMin")));
        modulatorFilterBank[i].prepare(spec);
                
        carrierFilterBank[i].reset();
        *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], *(parameters->getRawParameterValue("filterQMin")));
        carrierFilterBank[i].prepare(spec);
    }
}


void MPESynthVoice::updateParamRanges()
{
    float filterQMin = *parameters->getRawParameterValue("filterQMin");
    float filterQMax = *parameters->getRawParameterValue("filterQMax");
    float envFollowerMin = *parameters->getRawParameterValue("envFollowerMin");
    float envFollowerMax = *parameters->getRawParameterValue("envFollowerMax");
    float detuneMin = *parameters->getRawParameterValue("detuneMin");
    float detuneMax = *parameters->getRawParameterValue("detuneMax");
    float noiseMin = *parameters->getRawParameterValue("noiseMin");
    float noiseMax = *parameters->getRawParameterValue("noiseMax");
        
    if (filterQMin != filterQMax) {
        if (filterQMin < filterQMax) {
            this->filterQRange = NormalisableRange<float> (filterQMin, filterQMax);
        } else {
            this->filterQRange = NormalisableRange<float> (filterQMax, filterQMin);
            this->isFilterQRangeInverted = true;
        }
    }
    
    if (envFollowerMin != envFollowerMax) {
        if (envFollowerMin < envFollowerMax) {
            this->envFollowerRange = NormalisableRange<float> (envFollowerMin, envFollowerMax);
        } else {
            this->envFollowerRange = NormalisableRange<float> (envFollowerMax, envFollowerMin);
            this->isEnvFollowerRangeInverted = true;
        }
    }
    
    if (detuneMin != detuneMax) {
        if (detuneMin < detuneMax) {
            this->detuneRange = NormalisableRange<float> (detuneMin, detuneMax);
        } else {
            this->detuneRange = NormalisableRange<float> (detuneMax, detuneMin);
            this->isDetuneRangeInverted = true;
        }
    }
    
    if (noiseMin != noiseMax) {
        if (noiseMin < noiseMax) {
            this->noiseRange = NormalisableRange<float> (noiseMin, noiseMax);
        } else {
            this->noiseRange = NormalisableRange<float> (noiseMax, noiseMin);
            this->isNoiseRangeInverted = true;
        }
    }
}


void MPESynthVoice::modulateParamByController(int controlID, float intensity)
{
    int filterQControl =  *(parameters->getRawParameterValue("filterQControl"));
    if (filterQControl == controlID)
        modulateFilterQ(intensity);
    
    int envFollowerControl =  *(parameters->getRawParameterValue("envFollowerControl"));
    if (envFollowerControl == controlID)
        modulateEnvFollower(intensity);
    
    int detuneControl =  *(parameters->getRawParameterValue("detuneControl"));
    if (detuneControl == controlID)
        modulateDetune(intensity);
    
    int noiseControl = *(parameters->getRawParameterValue("noiseControl"));
    if (noiseControl == controlID)
        modulateNoise(intensity);
}


void MPESynthVoice::modulateFilterQ(float intensity)
{    
    if (isFilterQRangeInverted) intensity = 1 - intensity;
    
    float targetFilterQ = filterQRange.convertFrom0to1(intensity);
    currentFilterQ.setTargetValue(targetFilterQ);
    
    for (int i = 0; i < filterCount; ++i)
    {
        *modulatorFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], targetFilterQ);
        *carrierFilterBank[i].state = *dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, chosenFrequencies[i], targetFilterQ);
    }
}


void MPESynthVoice::modulateEnvFollower(float intensity)
{
    if (isEnvFollowerRangeInverted) intensity = 1 - intensity;
    
    float targetEnvFollow = envFollowerRange.convertFrom0to1(intensity);
    currentEnvFollowerFc.setTargetValue(targetEnvFollow);
    
    for (int i = 0; i < filterCount; ++i)
    {
        envFollowFilter[i]->setFc(targetEnvFollow);
    }
}


void MPESynthVoice::modulateDetune(float intensity)
{
    if (isDetuneRangeInverted) intensity = 1 - intensity;
    
    float targetDetune = detuneRange.convertFrom0to1(intensity);
    currentDetune.setTargetValue(targetDetune);
}


void MPESynthVoice::modulateNoise(float intensity)
{
    if (isNoiseRangeInverted) intensity = 1 - intensity;
    
    float targetNoiseMix = noiseRange.convertFrom0to1(intensity);
    currentNoiseMix.setTargetValue(targetNoiseMix);
}
