#include "PluginProcessor.h"
#include "PluginEditor.h"


Mpe_woksizerAudioProcessorEditor::Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
      processor (p),
      valueTreeState(vts)
{
    setSize (360, 200);
    
    volumeSlider.setSliderStyle(Slider::LinearBarVertical);
    volumeSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    volumeSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&volumeSlider);
    volumeAttachment.reset (new SliderAttachment (valueTreeState, "volume", volumeSlider));
    
    filterQSlider.setSliderStyle(Slider::LinearBarVertical);
    filterQSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    filterQSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&filterQSlider);
    filterQAttachment.reset (new SliderAttachment (valueTreeState, "filterQ", filterQSlider));
    
    onePoleFcSlider.setSliderStyle(Slider::LinearBarVertical);
    onePoleFcSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    onePoleFcSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&onePoleFcSlider);
    onePoleFcAttachment.reset (new SliderAttachment (valueTreeState, "onePoleFc", onePoleFcSlider));
    
    osc2DetuneSlider.setSliderStyle(Slider::LinearBarVertical);
    osc2DetuneSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    osc2DetuneSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&osc2DetuneSlider);
    osc2DetuneAttachment.reset (new SliderAttachment (valueTreeState, "osc2Detune", osc2DetuneSlider));
    
    oscNoiseLevelSlider.setSliderStyle(Slider::LinearBarVertical);
    oscNoiseLevelSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    oscNoiseLevelSlider.setPopupDisplayEnabled(true, false, this);
    addAndMakeVisible(&oscNoiseLevelSlider);
    oscNoiseLevelAttachment.reset (new SliderAttachment (valueTreeState, "oscNoiseLevel", oscNoiseLevelSlider));
}


Mpe_woksizerAudioProcessorEditor::~Mpe_woksizerAudioProcessorEditor()
{
}


void Mpe_woksizerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
    g.setColour(Colours::black);
    g.setFont (15.0f);
    g.drawSingleLineText("Volume", 10, 20);
    g.drawSingleLineText("Filter Q", 83, 20);
    g.drawSingleLineText("Env Follower", 144, 20);
    g.drawSingleLineText("Detune", 235, 20);
    g.drawSingleLineText("Noise", 313, 20);
}


void Mpe_woksizerAudioProcessorEditor::resized()
{
    int h = getHeight() - 60;
    volumeSlider.setBounds(20, 30, 20, h);
    filterQSlider.setBounds(95, 30, 20, h);
    onePoleFcSlider.setBounds(170, 30, 20, h);
    osc2DetuneSlider.setBounds(245, 30, 20, h);
    oscNoiseLevelSlider.setBounds(320, 30, 20, h);
}
