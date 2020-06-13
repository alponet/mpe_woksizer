#include "PluginProcessor.h"
#include "PluginEditor.h"


Mpe_woksizerAudioProcessorEditor::Mpe_woksizerAudioProcessorEditor (Mpe_woksizerAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
      processor (p),
      valueTreeState(vts)
{
    setSize (400, 200);
    
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
    g.drawSingleLineText("Vol", 20, 20);
    g.drawSingleLineText("Q", 153, 20);
    g.drawSingleLineText("Trans", 176, 20);
    g.drawSingleLineText("Tune", 308, 20);
    g.drawSingleLineText("Noise", 355, 20);
}


void Mpe_woksizerAudioProcessorEditor::resized()
{
    int h = getHeight() - 60;
    volumeSlider.setBounds(20, 30, 20, h);
    filterQSlider.setBounds(150, 30, 20, h);
    onePoleFcSlider.setBounds(180, 30, 20, h);
    osc2DetuneSlider.setBounds(310, 30, 20, h);
    oscNoiseLevelSlider.setBounds(360, 30, 20, h);
}
