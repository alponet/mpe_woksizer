#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


/**
*/
class Mpe_woksizerAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    
    Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor&, AudioProcessorValueTreeState&);
    ~Mpe_woksizerAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

private:
    Mpe_woksizerAudioProcessor& processor;
    AudioProcessorValueTreeState& valueTreeState;
    
    Slider volumeSlider;
    std::unique_ptr<SliderAttachment> volumeAttachment;
    
    Slider filterQMinSlider;
    std::unique_ptr<SliderAttachment> filterQMinAttachment;
    
    Slider filterQMaxSlider;
    std::unique_ptr<SliderAttachment> filterQMaxAttachment;
    
    ComboBox filterQControlBox;
    std::unique_ptr<ComboBoxAttachment> filterQControlAttachment;
    
    Slider envFollowerMinSlider;
    std::unique_ptr<SliderAttachment> envFollowerMinAttachment;
    
    Slider envFollowerMaxSlider;
    std::unique_ptr<SliderAttachment> envFollowerMaxAttachment;
    
    ComboBox envFollowerControlBox;
    std::unique_ptr<ComboBoxAttachment> envFollowerControlAttachment;
    
    Slider detuneMinSlider;
    std::unique_ptr<SliderAttachment> detuneMinAttachment;
    
    Slider detuneMaxSlider;
    std::unique_ptr<SliderAttachment> detuneMaxAttachment;
    
    ComboBox detuneControlBox;
    std::unique_ptr<ComboBoxAttachment> detuneControlAttachment;
    
    Slider noiseMinSlider;
    std::unique_ptr<SliderAttachment> noiseMinAttachment;
    
    Slider noiseMaxSlider;
    std::unique_ptr<SliderAttachment> noiseMaxAttachment;
    
    ComboBox noiseControlBox;
    std::unique_ptr<ComboBoxAttachment> noiseControlAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mpe_woksizerAudioProcessorEditor)
};
