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
    
    Slider filterQSlider;
    std::unique_ptr<SliderAttachment> filterQAttachment;
    
    Slider hpCutoffSlider;
    std::unique_ptr<SliderAttachment> hpCutoffAttachment;
    
    Slider hpQSlider;
    std::unique_ptr<SliderAttachment> hpQAttachment;
    
    Slider onePoleFcSlider;
    std::unique_ptr<SliderAttachment> onePoleFcAttachment;
    
    Slider osc1LevelSlider;
    std::unique_ptr<SliderAttachment> osc1LevelAttachment;
    
    Slider osc2LevelSlider;
    std::unique_ptr<SliderAttachment> osc2LevelAttachment;
    
    Slider osc2DetuneSlider;
    std::unique_ptr<SliderAttachment> osc2DetuneAttachment;
    
    Slider oscNoiseLevelSlider;
    std::unique_ptr<SliderAttachment> oscNoiseLevelAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mpe_woksizerAudioProcessorEditor)
};
