/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessorEditor::SimpleEQAudioProcessorEditor(SimpleEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), 
    peakGainSliderAttachement(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakFreqSliderAttachement(audioProcessor.apvts, "PeakCut Freq", peakFreqSlider),
    peakQualitySliderAttachement(audioProcessor.apvts,"Peak Quality", peakQualitySlider),
    lowCutFreqSliderAttachement(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
    highCutFreqSliderAttachement(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
    lowCutSlopeSliderAttachement(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    highCutSlopeSliderAttachement(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
    masterVolumeSliderAttachement(audioProcessor.apvts, "Master Volume", masterVolumeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    for (auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }


   

    setSize (1000, 500);
}

SimpleEQAudioProcessorEditor::~SimpleEQAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (50.0f);
    g.drawFittedText ("does it update?", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //bounding box for the components
    auto bounds = getLocalBounds();

    //dedicated area for future visualizer - cuts a third off the top
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    auto windowSpacing = bounds.removeFromBottom(bounds.getHeight() * 0.1);
   


    // THE ORDER ACTUALLY MATTERS FOR THE BOUND CALCULATIONS
    
    //Peak Gain Slider
    auto peakGainArea = bounds.removeFromLeft(bounds.getWidth() * 0.1);
    

    peakGainSlider.setBounds(peakGainArea);

    //Master Volume Slider
    auto masterVolumeArea = bounds.removeFromRight(bounds.getWidth() * 0.125);

    masterVolumeSlider.setBounds(masterVolumeArea);

    //Peak Frequency Slider
    
    int sliderWidth = getWidth() * 0.2;
    int sliderHeight = getHeight() * 0.125;
    int sliderXpos = getWidth() * 0.1;
    int sliderFreqYpos = getHeight() * 0.66;

   peakFreqSlider.setBounds(sliderXpos,sliderFreqYpos,sliderWidth,sliderHeight);


    //Peak Quality Slider

    peakQualitySlider.setBounds(sliderXpos, sliderFreqYpos - sliderHeight * 2 , sliderWidth, sliderHeight);

    //Low Cut Freq Knob

    int knobRadius = 100;

    lowCutFreqSlider.setBounds(sliderXpos * 2 + knobRadius , sliderFreqYpos - knobRadius / 3, knobRadius, knobRadius);

    //Low Cut Slope Knob


    lowCutSlopeSlider.setBounds(sliderXpos * 2 + knobRadius * 2, sliderFreqYpos - knobRadius / 3, knobRadius, knobRadius);

    //High Cut Freq Knob
    
    highCutFreqSlider.setBounds(sliderXpos * 2 + knobRadius, sliderFreqYpos - knobRadius - 10 - knobRadius / 3 , knobRadius, knobRadius);

    //High Cut Slope Knob

    highCutSlopeSlider.setBounds(sliderXpos * 2 + knobRadius * 2, sliderFreqYpos - knobRadius - 10 - knobRadius / 3, knobRadius, knobRadius);



 


}

std::vector<juce::Component*> SimpleEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &masterVolumeSlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}
