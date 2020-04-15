#include "PluginProcessor.h"
#include "PluginEditor.h"


Mpe_woksizerAudioProcessorEditor::Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
      processor (p),
      valueTreeState(vts)
{
    setSize (200, 200);
    
    volumeSlider.setSliderStyle(Slider::LinearBarVertical);
    volumeSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&volumeSlider);
    volumeAttachment.reset (new SliderAttachment (valueTreeState, "volume", volumeSlider));
    
    envAttackSlider.setSliderStyle(Slider::LinearBarVertical);
    envAttackSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    envAttackSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&envAttackSlider);
    envAttackAttachment.reset (new SliderAttachment (valueTreeState, "envAttack", envAttackSlider));
    
    envDecaySlider.setSliderStyle(Slider::LinearBarVertical);
    envDecaySlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    envDecaySlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&envDecaySlider);
    envDecayAttachment.reset (new SliderAttachment (valueTreeState, "envDecay", envDecaySlider));

    envSustainSlider.setSliderStyle(Slider::LinearBarVertical);
    envSustainSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    envSustainSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&envSustainSlider);
    envReleaseAttachment.reset (new SliderAttachment (valueTreeState, "envSustain", envSustainSlider));

    envReleaseSlider.setSliderStyle(Slider::LinearBarVertical);
    envReleaseSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    envReleaseSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&envReleaseSlider);
    envReleaseAttachment.reset (new SliderAttachment (valueTreeState, "envRelease", envReleaseSlider));
}


Mpe_woksizerAudioProcessorEditor::~Mpe_woksizerAudioProcessorEditor()
{
}


void Mpe_woksizerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
    g.setColour(Colours::black);
    g.setFont (15.0f);
    g.drawSingleLineText("Vol", 20, 20);
    g.drawSingleLineText("A", 76, 20);
    g.drawSingleLineText("D", 106, 20);
    g.drawSingleLineText("S", 136, 20);
    g.drawSingleLineText("R", 166, 20);
}


void Mpe_woksizerAudioProcessorEditor::resized()
{
    int h = getHeight() - 60;
    volumeSlider.setBounds(20, 30, 20, h);
    envAttackSlider.setBounds(70, 30, 20, h);
    envDecaySlider.setBounds(100, 30, 20, h);
    envSustainSlider.setBounds(130, 30, 20, h);
    envReleaseSlider.setBounds(160, 30, 20, h);
}
