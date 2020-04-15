/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
*/
class Mpe_woksizerAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    
    Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor&, AudioProcessorValueTreeState&);
    ~Mpe_woksizerAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    Mpe_woksizerAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    Slider volumeSlider;
    std::unique_ptr<SliderAttachment> volumeAttachment;
    
    Slider envAttackSlider;
    std::unique_ptr<SliderAttachment> envAttackAttachment;
    
    Slider envDecaySlider;
    std::unique_ptr<SliderAttachment> envDecayAttachment;
    
    Slider envSustainSlider;
    std::unique_ptr<SliderAttachment> envSustainAttachment;
    
    Slider envReleaseSlider;
    std::unique_ptr<SliderAttachment> envReleaseAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mpe_woksizerAudioProcessorEditor)
};
