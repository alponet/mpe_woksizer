#include "PluginProcessor.h"
#include "PluginEditor.h"


Mpe_woksizerAudioProcessorEditor::Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (200, 200);
    
    volumeSlider.setSliderStyle(Slider::LinearBarVertical);
    volumeSlider.setRange(0.0, 2.0, 0.01);
    volumeSlider.setSkewFactor(0.8);
    volumeSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    volumeSlider.setTextValueSuffix(" Volume");
    volumeSlider.setValue(1.0);
    
    volumeSlider.addListener(this);
    
    addAndMakeVisible(&volumeSlider);
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


void Mpe_woksizerAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
    processor.volume = volumeSlider.getValue();
}
