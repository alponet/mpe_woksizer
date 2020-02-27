#include "PluginProcessor.h"
#include "PluginEditor.h"


Mpe_woksizerAudioProcessorEditor::Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
      processor (p),
      valueTreeState(vts)
{
    setSize (200, 200);
    
    volumeSlider.setSliderStyle(Slider::LinearBarVertical);
    volumeSlider.setSkewFactor(0.8);
    volumeSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    volumeSlider.setTextValueSuffix(" Volume");
    
    addAndMakeVisible(&volumeSlider);
    volumeAttachment.reset (new SliderAttachment (valueTreeState, "volume", volumeSlider));
}


Mpe_woksizerAudioProcessorEditor::~Mpe_woksizerAudioProcessorEditor()
{
}


void Mpe_woksizerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
    g.setColour(Colours::black);
    g.setFont (15.0f);
    g.drawFittedText ("Volume", 0, 0, getWidth(), 30, Justification::centred, 1);
}


void Mpe_woksizerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    volumeSlider.setBounds(40, 30, 20, getHeight() - 60);
}
