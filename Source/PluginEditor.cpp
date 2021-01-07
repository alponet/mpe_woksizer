#include "PluginProcessor.h"
#include "PluginEditor.h"


Mpe_woksizerAudioProcessorEditor::Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
      processor (p),
      valueTreeState(vts)
{
    setSize (400, 220);
    
    volumeSlider.setSliderStyle(Slider::Rotary);
    volumeSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&volumeSlider);
    volumeAttachment.reset (new SliderAttachment (valueTreeState, "volume", volumeSlider));
    
    filterQMinSlider.setSliderStyle(Slider::Rotary);
    filterQMinSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&filterQMinSlider);
    filterQMinAttachment.reset (new SliderAttachment (valueTreeState, "filterQMin", filterQMinSlider));
    
    filterQMaxSlider.setSliderStyle(Slider::Rotary);
    filterQMaxSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&filterQMaxSlider);
    filterQMaxAttachment.reset (new SliderAttachment (valueTreeState, "filterQMax", filterQMaxSlider));

    filterQControlBox.addItemList(processor.controllerParams, 1);
    addAndMakeVisible(&filterQControlBox);
    filterQControlAttachment.reset (new ComboBoxAttachment(valueTreeState, "filterQControl", filterQControlBox));
    
    envFollowerMinSlider.setSliderStyle(Slider::Rotary);
    envFollowerMinSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&envFollowerMinSlider);
    envFollowerMinAttachment.reset (new SliderAttachment (valueTreeState, "envFollowerMin", envFollowerMinSlider));
    
    envFollowerMaxSlider.setSliderStyle(Slider::Rotary);
    envFollowerMaxSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&envFollowerMaxSlider);
    envFollowerMaxAttachment.reset (new SliderAttachment (valueTreeState, "envFollowerMax", envFollowerMaxSlider));

    envFollowerControlBox.addItemList(processor.controllerParams, 1);
    addAndMakeVisible(&envFollowerControlBox);
    envFollowerControlAttachment.reset (new ComboBoxAttachment(valueTreeState, "envFollowerControl", envFollowerControlBox));
    
    detuneMinSlider.setSliderStyle(Slider::Rotary);
    detuneMinSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&detuneMinSlider);
    detuneMinAttachment.reset (new SliderAttachment (valueTreeState, "detuneMin", detuneMinSlider));
    
    detuneMaxSlider.setSliderStyle(Slider::Rotary);
    detuneMaxSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&detuneMaxSlider);
    detuneMaxAttachment.reset (new SliderAttachment (valueTreeState, "detuneMax", detuneMaxSlider));
    
    detuneControlBox.addItemList(processor.controllerParams, 1);
    addAndMakeVisible(&detuneControlBox);
    detuneControlAttachment.reset (new ComboBoxAttachment(valueTreeState, "detuneControl", detuneControlBox));
    
    noiseMinSlider.setSliderStyle(Slider::Rotary);
    noiseMinSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&noiseMinSlider);
    noiseMinAttachment.reset (new SliderAttachment (valueTreeState, "noiseMin", noiseMinSlider));
    
    noiseMaxSlider.setSliderStyle(Slider::Rotary);
    noiseMaxSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(&noiseMaxSlider);
    noiseMaxAttachment.reset (new SliderAttachment (valueTreeState, "noiseMax", noiseMaxSlider));
    
    noiseControlBox.addItemList(processor.controllerParams, 1);
    addAndMakeVisible(&noiseControlBox);
    noiseControlAttachment.reset (new ComboBoxAttachment(valueTreeState, "noiseControl", noiseControlBox));
}


Mpe_woksizerAudioProcessorEditor::~Mpe_woksizerAudioProcessorEditor()
{
}


void Mpe_woksizerAudioProcessorEditor::paint (Graphics& g)
{
    getLookAndFeel().setColour(juce::Slider::thumbColourId, Colours::transparentBlack);
    getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, Colours::lightgrey);
    getLookAndFeel().setColour(juce::Slider::rotarySliderOutlineColourId, Colours::darkgrey);
    g.fillAll (Colours::black);
    g.setColour(Colours::lightgrey);
    g.setFont (14.0f);
    g.drawSingleLineText("Volume", 10, 160);
    g.drawSingleLineText("Filter Q", 90, 200);
    g.drawSingleLineText("Env Follow", 160, 200);
    g.drawSingleLineText("Detune", 252, 200);
    g.drawSingleLineText("Noise", 334, 200);
    g.setColour(Colours::darkgrey);
    g.setFont (11.0f);
    g.drawSingleLineText("max", 102, 106);
    g.drawSingleLineText("min", 102, 176);
    g.drawSingleLineText("max", 182, 106);
    g.drawSingleLineText("min", 182, 176);
    g.drawSingleLineText("max", 262, 106);
    g.drawSingleLineText("min", 262, 176);
    g.drawSingleLineText("max", 342, 106);
    g.drawSingleLineText("min", 342, 176);
}


void Mpe_woksizerAudioProcessorEditor::resized()
{
    volumeSlider.setBounds(2, 80, 60, 60);
    filterQControlBox.setBounds(74, 10, 76, 20);
    filterQMaxSlider.setBounds(82, 46, 60, 60);
    filterQMinSlider.setBounds(82, 116, 60, 60);
    envFollowerControlBox.setBounds(154, 10, 76, 20);
    envFollowerMaxSlider.setBounds(162, 46, 60, 60);
    envFollowerMinSlider.setBounds(162, 116, 60, 60);
    detuneControlBox.setBounds(234, 10, 76, 20);
    detuneMaxSlider.setBounds(242, 46, 60, 60);
    detuneMinSlider.setBounds(242, 116, 60, 60);
    noiseControlBox.setBounds(314, 10, 76, 20);
    noiseMaxSlider.setBounds(322, 46, 60, 60);
    noiseMinSlider.setBounds(322, 116, 60, 60);
}
